# python_bot/nlp_handler.py

import json
import httpx 
from telegram import Update
from telegram.ext import ContextTypes
import os
import sys
import time

from .kafka_service import send_telegram_update_to_kafka 
sys.path.append(os.path.dirname(os.path.abspath(__file__))) 

NLP_SERVICE_URL = os.getenv("NLP_SERVICE_URL", "http://localhost:8000/process_text")

http_client = httpx.AsyncClient()

async def nlp_text_handler(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """
    Обрабатывает текстовые сообщения от пользователя. Отправляет их в NLP-сервис,
    затем формирует стандартизированное Kafka-сообщение с event_type "telegram_message",
    используя готовую команду из NLP-паттернов.
    """
    if not (update.message and update.message.text):
        print("Received update without message text. Skipping NLP processing.")
        return

    user_text = update.message.text
    user_id = update.message.from_user.id
    username = update.message.from_user.username if update.message.from_user.username else ""
    first_name = update.message.from_user.first_name if update.message.from_user.first_name else ""
    chat_id = update.message.chat.id 

    print(f"Received message from user {user_id} in chat {chat_id}: '{user_text}'")

    simulated_telegram_command = "" # Это будет строка, имитирующая команду Telegram

    try:
        nlp_payload_to_service = {
            "user_id": user_id,
            "username": username,
            "first_name": first_name,
            "chat_id": chat_id,
            "text": user_text 
        }
        
        print(f"Sending to NLP service: {nlp_payload_to_service}")
        response = await http_client.post(NLP_SERVICE_URL, json=nlp_payload_to_service, timeout=5.0)
        response.raise_for_status()
        
        nlp_service_response = response.json()
        print(f"Received from NLP service: {nlp_service_response}")

        # matched_command теперь уже будет в формате "/weather", "/forecast" и т.д.
        matched_command = nlp_service_response.get("command") 
        nlp_entities = nlp_service_response.get("entities", {})

        if matched_command:
            simulated_telegram_command = matched_command # Берем команду напрямую
            
            # Добавляем сущности, если они есть
            if nlp_entities:
                # В первую очередь город, если он есть
                if "city" in nlp_entities and nlp_entities["city"]:
                    simulated_telegram_command += f" {nlp_entities['city']}"
                
                # Затем дата, если она есть и это команда для прогноза
                if "date" in nlp_entities and nlp_entities["date"] and matched_command == "/forecast":
                     simulated_telegram_command += f" {nlp_entities['date']}"
                
                # Добавь другие сущности, если их нужно встроить в строку команды
                # Например, для подписки:
                if matched_command in ["/subscribe", "/modify_subscription"]:
                    if "condition" in nlp_entities and nlp_entities["condition"]:
                        simulated_telegram_command += f" {nlp_entities['condition']}"
                    if "temp_above" in nlp_entities and nlp_entities["temp_above"] is not None:
                         simulated_telegram_command += f" temp_gt_{nlp_entities['temp_above']}"
                    if "rain" in nlp_entities and nlp_entities["rain"] is not None:
                         simulated_telegram_command += f" rain_{'true' if nlp_entities['rain'] else 'false'}"
                    if "wind_speed_gt" in nlp_entities and nlp_entities["wind_speed_gt"] is not None:
                         simulated_telegram_command += f" wind_gt_{nlp_entities['wind_speed_gt']}"
                    if "notify_time" in nlp_entities and nlp_entities["notify_time"]:
                         simulated_telegram_command += f" time_{nlp_entities['notify_time']}"

        else:
            # print(f"[DEBUG] text: {simulated_telegram_command}")
            simulated_telegram_command = "/unknown_nlp_query" 
            print(f"Warning: NLP service did not return a valid command for text: '{user_text}'. Using '{simulated_telegram_command}'.")
        
    except httpx.RequestError as exc:
        print(f"HTTP request error to NLP service for {exc.request.url!r}: {exc}")
        await context.bot.send_message(chat_id=chat_id, text="Простите, сервис обработки команд временно недоступен. Пожалуйста, попробуйте позже.")
        simulated_telegram_command = "/nlp_error_service_unavailable" 
    except httpx.HTTPStatusError as exc:
        print(f"Error response {exc.response.status_code} while requesting {exc.request.url!r}: {exc.response.text}")
        await context.bot.send_message(chat_id=chat_id, text="Простите, произошла ошибка при обработке вашей команды. Пожалуйста, попробуйте позже.")
        simulated_telegram_command = "/nlp_error_http_status"
    except json.JSONDecodeError:
        print(f"Error decoding JSON response from NLP service: {response.text}")
        await context.bot.send_message(chat_id=chat_id, text="Простите, не удалось разобрать ответ от сервиса. Пожалуйста, попробуйте позже.")
        simulated_telegram_command = "/nlp_error_json_decode"
    except Exception as e:
        print(f"An unexpected error occurred in nlp_text_handler: {e}")
        await context.bot.send_message(chat_id=chat_id, text="Произошла непредвиденная ошибка при обработке команды. Пожалуйста, попробуйте позже.")
        simulated_telegram_command = "/nlp_error_unexpected"

    kafka_message_payload = {
        "event_type": "telegram_message", 
        "timestamp": int(update.message.date.timestamp()) if update.message.date else int(time.time()),
        "source": "telegram_bot",
        "data": {
            "user": {
                "user_id": user_id,
                "username": username,
                "first_name": first_name,
                "chat_id": chat_id
            },
            "command": simulated_telegram_command, 
            "original_text": user_text,   
        }
    }

    send_telegram_update_to_kafka(kafka_message_payload)
    print(f"NLP-processed message (simulated command: '{simulated_telegram_command}') for user {user_id} sent to Kafka with event_type 'telegram_message'.")


async def close_http_client():
    await http_client.aclose()
    print("HTTP client for NLP service closed.")