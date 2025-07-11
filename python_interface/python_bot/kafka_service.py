# python_bot/kafka_service.py

import json
import asyncio
import os
import logging 
import time
from confluent_kafka import Producer, Consumer, KafkaError
from telegram.ext import Application

logger = logging.getLogger(__name__) 

kafka_producer: Producer = None
kafka_consumer: Consumer = None

telegram_app_instance: Application = None
KAFKA_COMMANDS_TOPIC: str = None
KAFKA_RESPONSES_TOPIC: str = None

def delivery_report(err, msg):
    """callback for kafka message delivery reports."""
    if err is not None:
        logger.error(f"message delivery failed: {err}")
    else:
        logger.debug(f"message delivered to {msg.topic()} [{msg.partition()}] @ offset {msg.offset()}")


def init_kafka(brokers: str, commands_topic: str, responses_topic: str, app_instance: Application):
    """initializes kafka producer and consumer."""
    global kafka_producer, kafka_consumer, telegram_app_instance, KAFKA_COMMANDS_TOPIC, KAFKA_RESPONSES_TOPIC

    telegram_app_instance = app_instance
    KAFKA_COMMANDS_TOPIC = commands_topic
    KAFKA_RESPONSES_TOPIC = responses_topic

    try:
        kafka_producer = Producer({'bootstrap.servers': brokers})
        logger.info(f"kafka producer connected to {brokers}, topic='{commands_topic}'")
    except Exception as e:
        logger.critical(f"error while connecting kafka producer: {e}")
        raise

    consumer_config = {
        'bootstrap.servers': brokers,
        'group.id': 'telegram_bot_response_group',
        'auto.offset.reset': 'earliest'
    }
    try:
        kafka_consumer = Consumer(consumer_config)
        kafka_consumer.subscribe([responses_topic])
        logger.info(f"kafka consumer subscribed to topic '{responses_topic}'")
    except Exception as e:
        logger.critical(f"error while connecting kafka consumer: {e}")
        raise

async def send_kafka_message(payload_dict: dict):
    """
    a universal function for sending kafka messages.
    accepts an already fully formed dictionary, ready for sending.
    """
    if kafka_producer is None or KAFKA_COMMANDS_TOPIC is None:
        logger.error("kafka producer or topic is not initialized. cannot send message.")
        return

    if "event_type" not in payload_dict or "data" not in payload_dict:
        logger.error(f"error: invalid kafka payload dict format. missing 'event_type' or 'data'. payload: {payload_dict}")
        return

    user_id = payload_dict.get("data", {}).get("user", {}).get("user_id")
    user_id_str = str(user_id) if user_id is not None else "unknown_user"

    event_type = payload_dict["event_type"]
    message_timestamp = payload_dict.get("timestamp", int(time.time()))

    kafka_payload = {
        "event_type": event_type,
        "timestamp": message_timestamp,
        "source": "telegram_bot",
        "data": payload_dict["data"]
    }

    try:
        json_value = json.dumps(kafka_payload, ensure_ascii=False).encode('utf-8')
        kafka_producer.produce(
            topic=KAFKA_COMMANDS_TOPIC,
            key=user_id_str.encode('utf-8'),
            value=json_value,
            callback=delivery_report
        )
        kafka_producer.poll(0)
        logger.info(f"sent event '{event_type}' from user '{user_id_str}' to kafka topic '{KAFKA_COMMANDS_TOPIC}'")
    except Exception as e:
        logger.error(f"error while sending to kafka: {e}. payload: {kafka_payload}")


async def kafka_response_listener(main_loop_for_telegram: asyncio.AbstractEventLoop):
    """
    listens for responses from kafka and sends them back to telegram.
    runs in a separate thread.
    """
    global kafka_consumer, telegram_app_instance
    if kafka_consumer is None or telegram_app_instance is None:
        logger.error("kafka consumer or application instance not initialized for listener.")
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
                    logger.error(f"error kafka consumer: {msg.error()}")
                    continue

            payload = msg.value().decode('utf-8')
            response_data = json.loads(payload)

            telegram_user_id = response_data.get('telegram_user_id')
            message_text = response_data.get('message')
            parse_mode = response_data.get('parse_mode', None)

            if telegram_user_id and message_text:
                logger.info(f"received response from kafka for user: {telegram_user_id}: {message_text[:18]}...")
                try:
                    send_coroutine = telegram_app_instance.bot.send_message(
                        chat_id=telegram_user_id,
                        text=message_text,
                        parse_mode=parse_mode
                    )
                    # run the coroutine in the main event loop
                    future = asyncio.run_coroutine_threadsafe(send_coroutine, main_loop_for_telegram)
                    # wait for the coroutine to complete
                    future.result()

                except Exception as e:
                    logger.error(f"error while sending to telegram api user {telegram_user_id}: {e}")
            else:
                logger.warning(f"incorrect format response from kafka: {response_data}")

        except json.JSONDecodeError as e:
            logger.error(f"parse error json from kafka: {e} | msg: {payload}")
        except Exception as e:
            logger.error(f"unexpected error in kafka listener: {e}")
        finally:
            # commit offsets asynchronously to acknowledge message processing
            kafka_consumer.commit(asynchronous=True)

def close_kafka_consumer():
    """closes the kafka consumer."""
    global kafka_consumer
    if kafka_consumer:
        kafka_consumer.close()
        logger.info("kafka consumer closed.")


async def close_kafka_producer():
    """
    gracefully closes the kafka producer (confluent_kafka).
    uses flush() to ensure all buffered messages are sent.
    """
    global kafka_producer
    if kafka_producer is not None:
        logger.info("closing kafka producer...")
        try:
            remaining_messages = kafka_producer.flush(timeout=5) 
            if remaining_messages > 0:
                logger.warning(f"failed to deliver {remaining_messages} messages before closing kafka producer.")
            logger.info("kafka producer closed successfully.")
        except Exception as e:
            logger.error(f"error closing kafka producer: {e}", exc_info=True)
        finally:
            kafka_producer = None 
    else:
        logger.info("kafka producer was not active or already closed.") 