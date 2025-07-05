# python_bot/commands.py

from telegram import BotCommand
from telegram.ext import Application 

BOT_COMMANDS = [
    BotCommand("start", "Начать работу с ботом"),
    BotCommand("weather", "Текущая погода: /weather <город>"),
    BotCommand("forecast", "Прогноз погоды: /forecast <город>"),
    BotCommand("mycity", "Сохранить/показать город по умолчанию: /mycity [город]"),
    BotCommand("subscribe", "Подписаться на уведомления: /subscribe <город> [время UTC]"),
    BotCommand("unsubscribe", "Отписаться от уведомлений: /unsubscribe <город> | all"),
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
    try:
        await app_instance.bot.set_my_commands(BOT_COMMANDS)
        print("Telegram commands set successfully.", flush=True)
    except Exception as e:
        print(f"Error setting Telegram commands: {e}", flush=True)