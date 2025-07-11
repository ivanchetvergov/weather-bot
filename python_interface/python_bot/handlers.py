# python_bot/handlers.py
import logging
from telegram import Update
from telegram.ext import ContextTypes

from .kafka_service import send_kafka_message
from .nlp_client import call_nlp_service
from .message_builder import build_kafka_payload
from .utils import escape_markdown_v2
from .config import OPENWEATHER_API_KEY 

logger = logging.getLogger(__name__)

async def _process_user_request(update: Update, 
                                 context: ContextTypes.DEFAULT_TYPE, 
                                 default_intent: str, # default intent if nlp fails or is not specific enough
                                 initial_reply_text: str # immediate reply to the user
                                 ) -> None:
    """
    a unified handler function to process various user requests (commands or text).
    it calls the nlp service, builds a kafka payload, sends it, and provides an immediate reply.
    """
    user_id = update.effective_user.id
    full_text = update.message.text
    logger.info(f"user {user_id} used command/text (via unified handler): '{full_text}'")

    # check if openweather api key is available for weather-related commands
    if default_intent in ["/weather", "/forecast", "/details", "/whattowear"] and not OPENWEATHER_API_KEY:
        logger.error("error: openweather_api_key not found in config.")
        await update.message.reply_text("an error occurred: weather api key not found\\.\\nplease contact the administrator\\.", parse_mode='MarkdownV2')
        return

    # 1. call the nlp service to process the user's text
    nlp_result = await call_nlp_service(
        user_id=user_id,
        username=update.effective_user.username or "",
        first_name=update.effective_user.first_name or "",
        chat_id=update.effective_chat.id,
        text=full_text
    )

    # 2. retrieve the intent and entities from the nlp result
    intent = nlp_result.get("command", default_intent) # use nlp's intent, or fallback to default
    entities = nlp_result.get("entities", {})
    # logger.info(f"processed text: {full_text}, intent: {intent}, entities:{entities}")

    # 3. build the kafka message payload
    payload = build_kafka_payload(
        update=update,
        event_type="telegram_command", # or "telegram_message" if this handler processes general text
        command_or_intent=intent,      # the recognized intent
        original_text=full_text,
        entities=entities              # the recognized entities
    )
    
    await send_kafka_message(payload)
    await update.message.reply_text(initial_reply_text)


async def start_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    user_id = update.effective_user.id
    logger.info(f"user {user_id} used /start command.")
    await update.message.reply_text("hi there! i'm your weather bot. to get the weather, use /weather <city>.")
    
    payload = build_kafka_payload(
        update=update, event_type="telegram_command", command_or_intent="/start", 
        original_text=update.message.text, entities={}
    )
    await send_kafka_message(payload)

async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    user_id = update.effective_user.id
    logger.info(f"user {user_id} requested /help.")
    
    from .commands import BOT_COMMANDS
    help_text = "list of available commands:\n\n"
    for cmd in BOT_COMMANDS:
        help_text += f"`\\/{escape_markdown_v2(cmd.command)}` \\- {escape_markdown_v2(cmd.description)}\n"
    
    await update.message.reply_text(help_text, parse_mode='MarkdownV2')
    
    payload = build_kafka_payload(
        update=update, event_type="telegram_command", command_or_intent="/help", 
        original_text=update.message.text, entities={}
    )
    await send_kafka_message(payload)

# --- all other commands now use the unified _process_user_request handler ---

async def weather_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/weather", "getting current weather for you...")

async def forecast_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/forecast", "getting weather forecast for you...")

async def mycity_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/mycity", "processing your default city request...")

async def subscribe_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/subscribe", "your subscription request is being processed. i'll let you know when it's set up.")

async def unsubscribe_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/unsubscribe", "cancelling your subscription...")

async def track_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/track", "your tracking request is being processed. i'll notify you when the condition is met.")

async def untrack_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/untrack", "cancelling your tracking...")

async def mysubscriptions_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/mysubscriptions", "retrieving your subscriptions list...")

async def mytracks_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/mytracks", "retrieving your tracking list...")

async def details_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/details", "retrieving detailed weather information...")

async def whattowear_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/whattowear", "preparing clothing recommendations...")

async def ask_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _process_user_request(update, context, "/ask", "forwarding your question for processing...")