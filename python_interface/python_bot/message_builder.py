# python_bot/message_builder.py

import time
from telegram import Update
from typing import Dict, Any, Optional

def build_kafka_payload(
    update: Update,
    event_type: str, 
    command_or_intent: str, # command name (/weather) 
    original_text: str,
    entities: Optional[Dict[str, Any]] = None # extracted nlp entities
) -> Dict[str, Any]:
    """
    constructs a dictionary representing a telegram event for sending to kafka.
    includes user data, original text, and extracted nlp entities.
    """
    user = update.effective_user 
    chat = update.effective_chat 

    user_id = user.id
    username = user.username if user.username else ""
    first_name = user.first_name if user.first_name else ""
    chat_id = chat.id

    # message timestamp if available, otherwise current time
    message_timestamp = int(update.effective_message.date.timestamp()) if update.effective_message and update.effective_message.date else int(time.time())

    kafka_payload = {
        "event_type": event_type,
        "timestamp": message_timestamp,
        "source": "telegram_bot", # source is always telegram bot
        "data": {
            "user": {
                "user_id": user_id,
                "username": username,
                "first_name": first_name,
                "chat_id": chat_id
            },
            "command": command_or_intent, 
            "original_text": original_text, # original text from the user
            "entities": entities if entities is not None else {} # nlp results, if any
        }
    }
    return kafka_payload