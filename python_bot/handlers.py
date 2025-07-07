# python_bot/handlers.py (исправлено)

from telegram import Update
from telegram.ext import ContextTypes
import json
import logging

from .kafka_service import send_telegram_update_to_kafka
from .commands import BOT_COMMANDS

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# ---
## Общая функция для отправки всех обновлений в Kafka

async def handle_all_updates_to_kafka(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """
    Отправляет все входящие обновления Telegram в Kafka.
    После отправки уведомляет пользователя о принятии запроса.
    """
    user_id = update.effective_user.id if update.effective_user else 'N/A'
    logger.info(f"Received update from user {user_id}. Sending to Kafka.")

    # Создаем структурированный объект для Kafka.
    # Добавляем 'type' поля для удобства обработки на бэкенде.
    kafka_message = {
        "type": "telegram_update", # Общий тип для всех обновлений Telegram
        "data": update.to_dict() # Полный объект обновления Telegram
    }
    send_telegram_update_to_kafka(kafka_message) # Отправляем этот объект

    if update.message:
        await update.message.reply_text("Ваш запрос принят и обрабатывается...")
    elif update.callback_query:
        await update.callback_query.answer("Запрос принят")

# ---
## Команда /weather

async def weather_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """
    Обрабатывает команду /weather.
    Отправляет запрос на получение погоды в Kafka для обработки бэкендом.
    """
    user_id = update.effective_user.id if update.effective_user else 'N/A'
    city = " ".join(context.args)

    if not city:
        logger.warning(f"User {user_id} sent /weather without city.")
        await update.message.reply_text("Пожалуйста, укажите город. Пример: /weather Москва")
        return

    logger.info(f"User {user_id} requested weather for {city}. Sending to Kafka.")

    # Здесь мы не отправляем специальный "command_weather" тип,
    # так как handle_all_updates_to_kafka уже отправит полное обновление.
    # Если ты хочешь, чтобы /weather был обработан отдельно,
    # то эту команду можно не регистрировать в handle_all_updates_to_kafka
    # и она будет вызывать send_telegram_update_to_kafka напрямую,
    # но тогда нужно убедиться, что _только_ она это делает.
    # Для простоты, предлагаю пока оставить так, что handle_all_updates_to_kafka обрабатывает _все_.

    # Если же ты хочешь, чтобы _только_ /weather отправлял "type": "command_weather",
    # то убери weather_command из `handle_all_updates_to_kafka` и измени:
    # kafka_message = {
    #     "type": "command_weather",
    #     "data": update.to_dict()
    # }
    # send_telegram_update_to_kafka(kafka_message)

    await update.message.reply_text("Запрос на получение погоды принят. Пожалуйста, ожидайте ответа.")

# ---
## Команда /start

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """
    Обрабатывает команду /start.
    Отправляет информацию о старте в Kafka для обработки бэкендом.
    """
    user_id = update.effective_user.id if update.effective_user else 'N/A'
    name = update.effective_user.first_name if update.effective_user else "друг"
    logger.info(f"User {user_id} sent /start. Sending to Kafka.")

    # Отправка обновления через handle_all_updates_to_kafka уже произошла.
    # Эта функция просто отвечает пользователю.
    await update.message.reply_text(f"Привет, {name}! Я твой погодный бот 🌦. Ваш запрос на старт отправлен.")

# ---
## Команда /help

async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """
    Обрабатывает команду /help.
    Отправляет пользователю список доступных команд.
    """
    user_id = update.effective_user.id if update.effective_user else 'N/A'
    logger.info(f"User {user_id} requested /help. Sent command list.")

    help_text = "Список доступных команд:\n\n"
    for cmd in BOT_COMMANDS:
        help_text += f"/{cmd.command} - {cmd.description}\n"

    await update.message.reply_text(help_text)