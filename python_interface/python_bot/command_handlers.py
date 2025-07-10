# python_bot/handlers.py

from telegram import Update
from telegram.ext import ContextTypes
from .kafka_service import send_telegram_update_to_kafka
from .commands import BOT_COMMANDS # Предполагается, что это существует

def _escape_markdown_v2(text: str) -> str:
    """Escapes characters for MarkdownV2."""
    escape_chars = r'_*[]()~`>#+-=|{}.!'
    return ''.join(['\\' + char if char in escape_chars else char for char in text])


async def weather_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    openweather_api_key = context.bot_data.get("OPENWEATHER_API_KEY")
    if not openweather_api_key:
        print("Error: OPENWEATHER_API_KEY not found in bot_data.")
        await update.message.reply_text("Произошла ошибка: ключ API погоды не найден\\.\\nПожалуйста, свяжитесь с администратором\\.", parse_mode='MarkdownV2')
        return

    await _send_telegram_command_to_kafka_with_original_text(update) 
    print(f"User {update.effective_user.id} requested weather for '{update.message.text}'. Request sent to Kafka.")


async def start_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _send_telegram_command_to_kafka_with_original_text(update)


async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    help_text = "Список доступных команд:\n\n"
    for cmd in BOT_COMMANDS:
        help_text += f"`\\/{_escape_markdown_v2(cmd.command)}` \\- {_escape_markdown_v2(cmd.description)}\n"

    await update.message.reply_text(help_text, parse_mode='MarkdownV2')
    await _send_telegram_command_to_kafka_with_original_text(update)
    print(f"User {update.effective_user.id} requested /help. Sent command list.")


async def forecast_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await _send_telegram_command_to_kafka_with_original_text(update) 
    print(f"User {update.effective_user.id} requested forecast for '{update.message.text}'. Request sent to Kafka.")



async def mycity_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    city_arg = " ".join(context.args)
    if not city_arg:
        await update.message.reply_text("Пожалуйста, укажите город по умолчанию\\. Пример: `/mycity Москва`", parse_mode='MarkdownV2')
        return

    await _send_telegram_command_to_kafka_with_original_text(update) 
    
    print(f"User {update.effective_user.id} requested to set default city to '{city_arg}'. Request sent to Kafka.")


async def _send_telegram_command_to_kafka_with_original_text(update: Update) -> None:
    """
    Формирует и отправляет Kafka-сообщение в твоем унифицированном формате,
    где 'command' и 'original_text' содержат полный текст сообщения Telegram.
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
            "command": original_message_text,     
            "original_text": original_message_text 
        }
    }
    
    send_telegram_update_to_kafka(kafka_message_payload)
    print(f"Unified Telegram message (command: '{original_message_text}', original: '{original_message_text}') from user {user_id} sent to Kafka.")