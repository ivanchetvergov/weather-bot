

import json
import asyncio
import os
from confluent_kafka import Producer, Consumer, KafkaException, KafkaError
from telegram import Update 
from telegram.ext import ApplicationBuilder 
import time
from datetime import datetime 
import logging
from typing import Tuple, Dict, Any, Optional

logger = logging.getLogger(__name__)

kafka_producer: Producer = None
kafka_consumer: Consumer = None
kafka_app_instance: ApplicationBuilder = None 
KAFKA_COMMANDS_TOPIC: str = None

def init_kafka(brokers: str, commands_topic: str, responses_topic: str, app_instance: ApplicationBuilder):
    global kafka_producer, kafka_consumer, kafka_app_instance, KAFKA_COMMANDS_TOPIC
    kafka_app_instance = app_instance 
    try:
        kafka_producer = Producer({'bootstrap.servers': brokers})
        KAFKA_COMMANDS_TOPIC = commands_topic
        print(f"Kafka Producer connected to {brokers}")
    except Exception as e:
        print(f"Error while connecting Kafka Producer: {e}")
        exit(1)

    consumer_config = {
        'bootstrap.servers': brokers,
        'group.id': 'telegram_bot_response_group',
        'auto.offset.reset': 'earliest'
    }
    try:
        kafka_consumer = Consumer(consumer_config)
        kafka_consumer.subscribe([responses_topic])
        print(f"Kafka Consumer subscribed to topic '{responses_topic}'")
    except Exception as e:
        print(f"Error while connecting Kafka Consumer: {e}")
        exit(1)

def delivery_report(err, msg):
    if err is not None:
        logger.error(f"Message delivery failed: {err}")
    else:
        logger.info(f"Message delivered to {msg.topic()} [{msg.partition()}] @ offset {msg.offset()}")

def _process_telegram_update(update: Update) -> Optional[Tuple[str, Dict[str, Any], int, str]]:
    """
    Processes a telegram.Update object and returns event_type, payload_container, timestamp, user_id_str.
    Returns None if the update is not supported.
    """
    if update.message: 
        user_id_str = str(update.message.from_user.id)
        event_type = "telegram_message"
        message_dict = update.message.to_dict()
        
        message_text = message_dict.get('text', '')
        command_text = message_text 

        message_timestamp = int(update.effective_message.date.timestamp()) if update.effective_message and update.effective_message.date else int(time.time())

        final_payload_data_container = {
            "message": {
                "id": message_dict.get('message_id'),
                "from": message_dict.get('from'),
                "chat_id": message_dict.get('chat').get('id') if message_dict.get('chat') else None,
                "date": message_dict.get('date'), # Telegram date is in Unix timestamp
                "text": message_text,
                "command_text": command_text,
            }
        }
        return event_type, final_payload_data_container, message_timestamp, user_id_str

    elif update.callback_query:
        user_id_str = str(update.callback_query.from_user.id)
        event_type = "telegram_callback_query"
        callback_dict = update.callback_query.to_dict()
        callback_data = callback_dict.get('data', '')

        message_timestamp = int(update.callback_query.message.date.timestamp()) if update.callback_query.message and update.callback_query.message.date else int(time.time())

        final_payload_data_container = {
            "callback_query": {
                "id": callback_dict.get('id'),
                "from": callback_dict.get('from'),
                "chat_id": callback_dict.get('message', {}).get('chat', {}).get('id'),
                "data": callback_data,
                "date": callback_dict.get('message', {}).get('date'),
                "command_text": callback_data # Default for callback queries
            }
        }
        return event_type, final_payload_data_container, message_timestamp, user_id_str
    else:
        logger.warning(f"Update object without message or callback_query. Skipping: {update.update_id}")
        return None

def _process_custom_dict(data_dict: dict) -> Optional[Tuple[str, Dict[str, Any], int, str]]:
    """
    Processes a custom dictionary (e.g., from NLP handler) and returns event_type, payload_container, timestamp, user_id_str.
    Returns None if the dictionary format is not supported.
    """
    event_type = data_dict.get("event_type", "custom_log_event")
    final_payload_data_container = data_dict.get("data", {}) 
    message_timestamp = int(time.time())
    user_id_str = "unknown"

    if "message" in final_payload_data_container and "from" in final_payload_data_container["message"]:
        user_id_str = str(final_payload_data_container["message"]["from"].get("id", "unknown_user"))
    elif "callback_query" in final_payload_data_container and "from" in final_payload_data_container["callback_query"]:
        user_id_str = str(final_payload_data_container["callback_query"]["from"].get("id", "unknown_user"))
    else:
        user_id_str = str(data_dict.get("user_id", "unknown_user")) 


    if "timestamp" in data_dict:
        try:
            if isinstance(data_dict["timestamp"], datetime):
                message_timestamp = int(data_dict["timestamp"].timestamp())
            elif isinstance(data_dict["timestamp"], (int, float)):
                message_timestamp = int(data_dict["timestamp"])
            elif isinstance(data_dict["timestamp"], str):
                message_timestamp = int(datetime.fromisoformat(data_dict["timestamp"]).timestamp())
        except (AttributeError, ValueError) as e:
            logger.error(f"Error parsing timestamp from dict: {e}")
            pass # Keep default time.time()

    return event_type, final_payload_data_container, message_timestamp, user_id_str

def send_telegram_update_to_kafka(data_to_send: Update | dict):
    """
    Sends a Telegram update or custom dictionary as a Kafka message.
    The data is formatted to include 'text' (original message) and 'command_text' (processed command).
    """
    global kafka_producer, KAFKA_COMMANDS_TOPIC
    if kafka_producer is None or KAFKA_COMMANDS_TOPIC is None:
        logger.error("Kafka Producer or Topic is not initialized. Cannot send message.")
        return

    result = None
    if isinstance(data_to_send, Update):
        result = _process_telegram_update(data_to_send)
    elif isinstance(data_to_send, dict):
        result = _process_custom_dict(data_to_send)
    else:
        logger.error(f"Unsupported data type for Kafka: {type(data_to_send)}. Must be Update or dict.")
        return

    if result is None:
        return 

    event_type, final_payload_data_container, message_timestamp, user_id_str = result

    kafka_payload = {
        "event_type": event_type,
        "timestamp": message_timestamp,
        "source": "telegram_bot",
        "data": final_payload_data_container 
    }

    try:
        kafka_producer.produce(
            KAFKA_COMMANDS_TOPIC,
            key=user_id_str.encode('utf-8'),
            value=json.dumps(kafka_payload, ensure_ascii=False).encode('utf-8'),
            callback=delivery_report
        )
        kafka_producer.poll(0) 
    
        logger.info(f"Sent event '{event_type}' from user '{user_id_str}' to Kafka topic '{KAFKA_COMMANDS_TOPIC}'")
    except Exception as e:
        logger.error(f"Error while sending to Kafka: {e}")

async def kafka_response_listener(main_loop_for_telegram): 
    global kafka_consumer, kafka_app_instance 
    if kafka_consumer is None or kafka_app_instance is None:
        print("Kafka Consumer or Application instance not initialized for listener.")
        return
    
    while True:
        try:
            msg = kafka_consumer.poll(timeout=1.0) 
            if msg is None:
                await asyncio.sleep(0.1) 
                continue
            if msg.error():
                if msg.error().code() == KafkaError._PARTITION_EOF:
                    continue
                else:
                    print(f"Error Kafka Consumer: {msg.error()}")
                    continue

            payload = msg.value().decode('utf-8')
            response_data = json.loads(payload)

            telegram_user_id = response_data.get('telegram_user_id')
            message_text = response_data.get('message')
            parse_mode = response_data.get('parse_mode', None)

            if telegram_user_id and message_text:
                print(f"Received response from Kafka for user: {telegram_user_id}: {message_text[:24]}...\n")
                try:
                    send_coroutine = kafka_app_instance.bot.send_message(
                        chat_id=telegram_user_id,
                        text=message_text,
                        parse_mode=parse_mode
                    )
                    future = asyncio.run_coroutine_threadsafe(send_coroutine, main_loop_for_telegram)
                    future.result() 

                except Exception as e:
                    print(f"Error while sending to Telegram API user {telegram_user_id}: {e}")
            else:
                print(f"Incorrect format response from Kafka: {response_data}")

        except json.JSONDecodeError as e:
            print(f"Parse error JSON from Kafka: {e} | Msg: {payload}")
        except Exception as e:
            print(f"Unexpected error in Kafka listener: {e}")
        finally:
            kafka_consumer.commit(asynchronous=True)

def close_kafka_consumer():
    global kafka_consumer
    if kafka_consumer:
        kafka_consumer.close()