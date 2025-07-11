# python_bot/commands.py

import logging
from telegram import BotCommand
from telegram.ext import Application

logger = logging.getLogger(__name__)

# Список команд для установки в Telegram
BOT_COMMANDS = [
    BotCommand("start", "Начать работу с ботом"),
    BotCommand("weather", "Текущая погода: /weather <город>"),
    BotCommand("forecast", "Прогноз погоды: /forecast <город>"),
    BotCommand("mycity", "Управлять городом по умолчанию: /mycity [город]"),
    BotCommand("subscribe", "Подписаться на уведомления: /subscribe <город> [условия]"),
    BotCommand("unsubscribe", "Отменить подписку: /unsubscribe <город> | all"),
    BotCommand("mysubscriptions", "Показать мои подписки"),
    BotCommand("track", "Отслеживать условие погоды: /track <город> <условие>"),
    BotCommand("untrack", "Отменить отслеживание: /untrack <id> | all"),
    BotCommand("mytracks", "Показать мои отслеживания"),
    BotCommand("details", "Детальная погода: /details <город>"),
    BotCommand("whattowear", "Совет по одежде: /whattowear <город>"),
    BotCommand("ask", "Задать вопрос боту (погода, общие запросы)"),
    BotCommand("help", "Показать список команд")
]

async def set_telegram_commands(app_instance: Application):
    """Sets the list of commands for the Telegram bot."""
    try:
        await app_instance.bot.set_my_commands(BOT_COMMANDS)
        logger.info("Telegram commands set successfully.")
    except Exception as e:
        logger.error(f"Error setting Telegram commands: {e}")