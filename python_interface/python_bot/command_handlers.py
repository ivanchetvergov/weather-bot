# python_bot/handlers.py

from telegram import Update
from telegram.ext import ContextTypes
from .kafka_service import send_telegram_update_to_kafka
from .commands import BOT_COMMANDS


import json

async def weather_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    openweather_api_key = context.bot_data.get("OPENWEATHER_API_KEY")
    if not openweather_api_key:
        print("Error: OPENWEATHER_API_KEY not found in bot_data.")
        return

    city = " ".join(context.args)
    if not city:
        await update.message.reply_text("Пожалуйста, укажите город. Пример: /weather Москва")
        return

    send_telegram_update_to_kafka(update) 
    print(f"User {update.effective_user.id} requested weather for {update.message.text}. Request sent to Kafka.")

async def start_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    send_telegram_update_to_kafka(update)

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

    send_telegram_update_to_kafka(update) 
    
    print(f"User {update.effective_user.id} requested forecast for {update.message.text}. Request sent to Kafka.")
