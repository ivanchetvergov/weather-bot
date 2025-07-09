# python_bot/nlp_handler.py

import json
from telegram import Update
from telegram.ext import ContextTypes
from datetime import datetime, timedelta

from nlp_spacy.nlp_service import NlpService

from .kafka_service import send_telegram_update_to_kafka

nlp_processor: NlpService = None

def set_nlp_processor(given: NlpService):
    global nlp_processor
    nlp_processor = given
    print("NLP processor set in nlp_handler.")


async def nlp_message_handler(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:

    if not update.message or not update.message.text or not nlp_processor:
        return

    user_id = update.effective_user.id
    username = update.effective_user.username
    first_name = update.effective_user.first_name
    message_text = update.message.text

    print(f"[{datetime.now().strftime('%H:%M:%S')}] NLP Handler: Received text from {username} ({user_id}): '{message_text}'")

    nlp_result = nlp_processor.process_text(message_text)

    data_to_kafka = {
        "event_type": "telegram_message", 
        "user_id": user_id,
        "username": username,
        "first_name": first_name,
        "message_text": message_text,
        "source": "natural_language_input"
    }

    if nlp_result:
        data_to_kafka["nlp_intent"] = nlp_result["intent"]
        data_to_kafka["nlp_entities"] = nlp_result["entities"]
        print(f"[{datetime.now().strftime('%H:%M:%S')}] NLP recognized intent: '{nlp_result['intent']}' with entities: {nlp_result['entities']}")
        await update.message.reply_text("Ваш запрос на естественном языке принят и обрабатывается...")
    else:
        data_to_kafka["nlp_intent"] = "UNKNOWN"
        data_to_kafka["nlp_entities"] = {}
        data_to_kafka["source"] = "unrecognized_natural_language"
        print(f"[{datetime.now().strftime('%H:%M:%S')}] NLP did not recognize any intent for: '{message_text}'")
        await update.message.reply_text("Извините, я не понял вашу команду. Пожалуйста, попробуйте перефразировать или используйте /help.")
    
    send_telegram_update_to_kafka(data_to_kafka)


