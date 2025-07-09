

import json
import asyncio
import os
from confluent_kafka import Producer, Consumer, KafkaException, KafkaError
from telegram import Update 
from telegram.ext import ApplicationBuilder 
import time
from datetime import datetime 

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
        print(f"Sending error to Kafka: {err}")

def send_telegram_update_to_kafka(data_to_send: Update | dict):
    global kafka_producer, KAFKA_COMMANDS_TOPIC
    if kafka_producer is None or KAFKA_COMMANDS_TOPIC is None:
        print("Kafka Producer or Topic is not initialized. Cannot send message.")
        return

    user_id_str = "unknown"
    event_type = "unknown"
    final_payload_data = None 
    message_timestamp = int(time.time())

    if isinstance(data_to_send, Update):
        if data_to_send.message: 
            user_id_str = str(data_to_send.message.from_user.id)
            event_type = "telegram_message"
            final_payload_data = {
                "message": data_to_send.message.to_dict()
            }
            if data_to_send.effective_message and data_to_send.effective_message.date:
                message_timestamp = int(data_to_send.effective_message.date.timestamp())
        elif data_to_send.callback_query: 
            user_id_str = str(data_to_send.callback_query.from_user.id)
            event_type = "telegram_callback_query"
            final_payload_data = {
                "callback_query": data_to_send.callback_query.to_dict() 
            }
            if data_to_send.callback_query.message and data_to_send.callback_query.message.date:
                message_timestamp = int(data_to_send.callback_query.message.date.timestamp())
        else:
            print(f"Update object without message or callback_query. Skipping: {data_to_send.update_id}")
            return
    elif isinstance(data_to_send, dict):
        if "event_type" in data_to_send:
            event_type = data_to_send["event_type"]
            final_payload_data = data_to_send

    
        elif "message" in data_to_send:
            event_type = "telegram_message"
            final_payload_data = data_to_send 

        elif "command" in data_to_send:
            event_type = "telegram_message" 

            temp_user_id = data_to_send.get("user_id", 0)
            temp_username = data_to_send.get("username", "")
            temp_first_name = data_to_send.get("first_name", "Unknown")
            temp_message_text = data_to_send.get("message_text", data_to_send["command"]) 

            final_payload_data = {
                "message": {
                    "from": {
                        "id": temp_user_id,
                        "username": temp_username,
                        "first_name": temp_first_name
                    },
                    "text": temp_message_text
                }
            }
            print(f"Warning: Dict with 'command' key ('{data_to_send['command']}') was transformed into 'telegram_message' event type to match C++ backend expectations.")
        else:
            event_type = "custom_log_event"
            final_payload_data = data_to_send

        user_id_str = str(data_to_send.get("user_id", "unknown_user"))

        if data_to_send.get("timestamp"):
            try:
                if isinstance(data_to_send["timestamp"], datetime):
                    message_timestamp = int(data_to_send["timestamp"].timestamp())
                elif isinstance(data_to_send["timestamp"], (int, float)):
                    message_timestamp = int(data_to_send["timestamp"])
                elif isinstance(data_to_send["timestamp"], str):
                    message_timestamp = int(datetime.fromisoformat(data_to_send["timestamp"]).timestamp())
            except (AttributeError, ValueError):
                pass

    else:
        print(f"Unsupported data type for Kafka: {type(data_to_send)}. Must be Update or dict.")
        return

    kafka_payload = {
        "event_type": event_type,
        "timestamp": message_timestamp,
        "source": "telegram_bot",
        "data": final_payload_data 
    }

    try:
        kafka_producer.produce(
            KAFKA_COMMANDS_TOPIC,
            key=user_id_str.encode('utf-8'),
            value=json.dumps(kafka_payload, ensure_ascii=False).encode('utf-8'),
            callback=delivery_report
        )
        kafka_producer.poll(0)
    
        print(f"Sent event '{event_type}' from user '{user_id_str}' to Kafka topic '{KAFKA_COMMANDS_TOPIC}'")
    except Exception as e:
        print(f"Error while sending to Kafka: {e}")

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