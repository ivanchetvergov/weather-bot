# python_bot/kafka_service.py

import json
import asyncio
import os
from confluent_kafka import Producer, Consumer, KafkaError
from telegram import Update
import time 

from telegram.ext import ApplicationBuilder 


kafka_producer: Producer = None
kafka_consumer: Consumer = None
kafka_app_instance: ApplicationBuilder = None 
KAFKA_COMMANDS_TOPIC: str = None

def delivery_report(err, msg):
    if err is not None:
        print(f"Sending error to Kafka: {err}")
    else:
        print(f"Message delivered to {msg.topic()} [{msg.partition()}] @ offset {msg.offset()}")


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

def send_telegram_update_to_kafka(kafka_payload_dict: dict):
    """
    Универсальная функция для отправки Kafka-сообщений.
    Принимает уже полностью сформированный словарь, готовый к отправке.
    Словарь должен содержать "event_type", "timestamp" и "data" 
    со всеми необходимыми полями, включая user_id, chat_id и command/original_text.
    """
    global kafka_producer, KAFKA_COMMANDS_TOPIC
    
    if kafka_producer is None or KAFKA_COMMANDS_TOPIC is None:
        print("Kafka Producer or Topic is not initialized. Cannot send message.")
        return

    # Проверяем, что необходимые поля для верхнего уровня присутствуют
    if "event_type" not in kafka_payload_dict or "data" not in kafka_payload_dict:
        print(f"Error: Invalid Kafka payload dict format. Missing 'event_type' or 'data' at top level. Payload: {kafka_payload_dict}")
        return

    # Безопасное извлечение user_id для ключа Kafka
    # Ожидается, что user_id находится в kafka_payload_dict["data"]["user"]["user_id"]
    user_id = kafka_payload_dict.get("data", {}).get("user", {}).get("user_id")
    if user_id is None:
        print(f"Warning: Kafka payload does not contain 'data.user.user_id'. Using 'unknown_user' as key. Payload: {kafka_payload_dict}")
        user_id_str = "unknown_user"
    else:
        user_id_str = str(user_id)
        
    event_type = kafka_payload_dict["event_type"] # event_type должен быть гарантированно после проверки выше


    message_timestamp = kafka_payload_dict.get("timestamp", int(time.time()))

    kafka_payload = {
        "event_type": event_type,
        "timestamp": message_timestamp,
        "source": "telegram_bot", # Источник всегда Telegram бот
        "data": kafka_payload_dict["data"] # Берем уже сформированный блок "data"
    }

    try:
        kafka_producer.produce(
            topic=KAFKA_COMMANDS_TOPIC,
            key=user_id_str.encode('utf-8'),
            value=json.dumps(kafka_payload, ensure_ascii=False).encode('utf-8'),
            callback=delivery_report
        )
        kafka_producer.poll(0) # Инициируем отправку
    
        print(f"Sent event '{event_type}' from user '{user_id_str}' to Kafka topic '{KAFKA_COMMANDS_TOPIC}'")
    except Exception as e:
        print(f"Error while sending to Kafka: {e}. Payload: {kafka_payload}")

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
                print(f"Received response from Kafka for user: {telegram_user_id}: {message_text[:18]}...\n")
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