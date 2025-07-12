# python_bot/nlp_text_handler.py

import logging
import json
from telegram import Update
from telegram.ext import ContextTypes

from nlp_spacy.config import CITY_NORMALIZATION, CONDITION_NORMALIZATION 

from .kafka_service import send_kafka_message
from .nlp_client import call_nlp_service
from .message_builder import build_kafka_payload

logger = logging.getLogger(__name__)

async def process_nlp_text_message(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """
    handles general text messages that are not telegram commands.
    it uses an nlp service to determine intent and extract entities, then sends the data to kafka.
    """
    if not (update.message and update.message.text):
        logger.warning("received update without message text. skipping nlp processing.")
        return

    user_text = update.message.text
    user_id = update.effective_user.id
    chat_id = update.effective_chat.id 
    print(f"\n" + "="*80 + "\n")
    logger.info(f"processing non-command text message from user {user_id} in chat {chat_id}: '{user_text}'")

    # call the external nlp service to process the user's text
    nlp_result = await call_nlp_service(
        user_id=user_id,
        username=update.effective_user.username or "",
        first_name=update.effective_user.first_name or "",
        chat_id=chat_id,
        text=user_text
    )
    
    # the nlp service returns the recognized intent (e.g., "get_weather") and extracted entities
    if nlp_result is None:
        logger.error(f"nlp service call returned None for user {user_id}, text: '{user_text}'")
        intent = "nlp_error" # Default to an error intent
        entities = {}        # Default to an empty dictionary
    else:
        intent = nlp_result.get("command", "unknown_text") 
        # Ensure entities is always a dictionary, even if nlp_result.get("entities") returns None
        entities = nlp_result.get("entities", {}) or {} # Added `or {}` for extra safety

    #  normalization to recognized entities
    if "city" in entities and entities["city"]:
        current_city_text = entities["city"].lower() # get the city text in lowercase
        # apply normalization: replace with canonical name if found, otherwise keep original
        entities["city"] = CITY_NORMALIZATION.get(current_city_text, entities["city"])

    if "condition" in entities and entities["condition"]:
        current_condition_text = entities["condition"].lower()
        entities["condition"] = CONDITION_NORMALIZATION.get(current_condition_text, entities["condition"])

    logger.info(f"nlp recognized intent: {intent}, normalized entities: {entities}")


    payload = build_kafka_payload(
        update=update,
        event_type="telegram_command", # type of event: general message
        command_or_intent=intent,      # what nlp recognized as the intent
        original_text=user_text,
        entities=entities             # pass the (now normalized) entities
    )
    logger.info(f"kafka payload built: {json.dumps(payload, ensure_ascii=False)}")
    await send_kafka_message(payload)

    # --- immediate responses ---
    if intent == "greeting":
        await update.message.reply_text("hi there! how can i help you today?")
    elif intent in ["/weather", "/forecast"]:
        await update.message.reply_text("Принял ваш запрос на погоду. обрабатываю...")
    elif intent == "nlp_error":
         await update.message.reply_text("Простите, произошла ошибка при обработке вашей команды. Пожалуйста, попробуйте позже.")
    else:
        pass