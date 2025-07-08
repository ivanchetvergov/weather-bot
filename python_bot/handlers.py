# python_bot/handlers.py

from telegram import Update
from telegram.ext import ContextTypes
from .kafka_service import send_telegram_update_to_kafka
from .commands import BOT_COMMANDS


import json

async def handle_all_updates_to_kafka(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    send_telegram_update_to_kafka(update)
    if update.message:
        await update.message.reply_text("Ваш запрос принят и обрабатывается...")
    elif update.callback_query:
        await update.callback_query.answer("Запрос принят")

async def weather_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    openweather_api_key = context.bot_data.get("OPENWEATHER_API_KEY")
    if not openweather_api_key:
        print("Error: OPENWEATHER_API_KEY not found in bot_data.")
        return

    city = " ".join(context.args)
    if not city:
        await update.message.reply_text("Пожалуйста, укажите город. Пример: /weather Москва")
        return

    log_data = {
        "user_id": update.effective_user.id,
        "username": update.effective_user.username,
        "first_name": update.effective_user.first_name,
        "command": "/weather",
        "city": city,
        "timestamp": update.message.date.isoformat(),
    }
    send_telegram_update_to_kafka(log_data)
    print(f"User {update.effective_user.id} requested weather for {city}. Log sent to Kafka.")


async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    send_telegram_update_to_kafka(update)
    name = update.effective_user.first_name if update.effective_user else "друг"
    await update.message.reply_text(f"Привет, {name}! Я твой погодный бот 🌦. Ваш запрос на старт отправлен.")

async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    help_text = "Список доступных команд:\n\n"
    for cmd in BOT_COMMANDS:
        help_text += f"/{cmd.command} - {cmd.description}\n"

    await update.message.reply_text(help_text)
    print(f"User {update.effective_user.id} requested /help. Sent command list.")
