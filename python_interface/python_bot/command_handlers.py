# python_bot/handlers.py

from telegram import Update
from telegram.ext import ContextTypes
from .kafka_service import send_telegram_update_to_kafka
from .commands import BOT_COMMANDS

async def weather_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    openweather_api_key = context.bot_data.get("OPENWEATHER_API_KEY")
    if not openweather_api_key:
        print("Error: OPENWEATHER_API_KEY not found in bot_data.")
        return

    city = " ".join(context.args)
    if not city:
        await update.message.reply_text("Пожалуйста, укажите город. Пример: /weather Москва")
        return

    await _send_unified_telegram_message_to_kafka(update, "weather") 
    print(f"User {update.effective_user.id} requested weather for {update.message.text}. Request sent to Kafka.")

async def start_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _send_unified_telegram_message_to_kafka(update, "start")

async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    help_text = "Список доступных команд:\n\n"
    for cmd in BOT_COMMANDS:
        help_text += f"/{cmd.command} - {cmd.description}\n"

    await update.message.reply_text(help_text)
    print(f"User {update.effective_user.id} requested /help. Sent command list.")

async def forecast_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:

    city = " ".join(context.args)

    if not city:
        await update.message.reply_text("Пожалуйста, укажите город для прогноза. Пример: /forecast Москва")
        return

    await _send_simple_telegram_command_to_kafka(update, "forecast") 
    
    print(f"User {update.effective_user.id} requested forecast for {update.message.text}. Request sent to Kafka.")

async def _send_unified_telegram_message_to_kafka(update: Update, context: ContextTypes.DEFAULT_TYPE | None = None, is_command: bool = True) -> None:
    """
    Формирует и отправляет унифицированное Kafka-сообщение для всех типов входящих сообщений Telegram.
    Использует event_type="telegram_message" с дополнительными полями для команды/NLP.
    """
    if not update.message:
        print("Warning: Update without message. Skipping Kafka send.")
        return

    user = update.message.from_user
    chat = update.message.chat
    
    user_id = user.id
    username = user.username if user.username else ""
    first_name = user.first_name if user.first_name else ""
    chat_id = chat.id

    original_message_text = update.message.text if update.message.text else ""
    
    command_or_intent = ""
    if is_command and original_message_text.startswith('/'):
        command_or_intent = original_message_text
    elif not is_command:
        command_or_intent = "nlp_text_query" 

    kafka_message_payload = {
        "event_type": "telegram_message", 
        "source": "telegram_bot",
        "data": {
            "user": { 
                "user_id": user_id,
                "username": username,
                "first_name": first_name,
                "chat_id": chat_id 
            },
            "command": command_or_intent,     
            "original_text": original_message_text 

        }
    }
    
    send_telegram_update_to_kafka(kafka_message_payload)
    print(f"Unified Telegram message ('{command_or_intent}') from user {user_id} sent to Kafka.")