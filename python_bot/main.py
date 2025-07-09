# python_bot/main.py

import os
import threading
import asyncio
from dotenv import load_dotenv
from telegram.ext import ApplicationBuilder, CommandHandler, MessageHandler, filters, CallbackQueryHandler

from .kafka_service import init_kafka, kafka_response_listener, close_kafka_consumer
from .handlers import *
from .commands import set_telegram_commands

async def post_init_setup(application):
    await set_telegram_commands(application) 

if __name__ == "__main__":
    load_dotenv()

    BOT_TOKEN = os.getenv("TELEGRAM_BOT_TOKEN")
    KAFKA_BROKERS = os.getenv("KAFKA_BROKERS")
    KAFKA_COMMANDS_TOPIC = os.getenv("KAFKA_COMMANDS_TOPIC")
    KAFKA_RESPONSES_TOPIC = os.getenv("KAFKA_RESPONSES_TOPIC")
    OPENWEATHER_API_KEY = os.getenv("OPENWEATHER_API_KEY")

    if not all([BOT_TOKEN, KAFKA_BROKERS, KAFKA_COMMANDS_TOPIC, KAFKA_RESPONSES_TOPIC, OPENWEATHER_API_KEY]):
        print("Error. Not all environment variables loaded.")
        exit(1)

    app = ApplicationBuilder().token(BOT_TOKEN).post_init(post_init_setup).build()
    app.bot_data["OPENWEATHER_API_KEY"] = OPENWEATHER_API_KEY

    main_async_loop = asyncio.get_event_loop() 

    init_kafka(KAFKA_BROKERS, KAFKA_COMMANDS_TOPIC, KAFKA_RESPONSES_TOPIC, app)

    def run_kafka_listener_in_thread(app_instance, loop_to_use): 
        loop = asyncio.new_event_loop() 
        asyncio.set_event_loop(loop) 
        try:
            loop.run_until_complete(kafka_response_listener(loop_to_use)) 
        except asyncio.CancelledError:
            pass
        finally:
            loop.close()

    listener_thread = threading.Thread(target=run_kafka_listener_in_thread, args=(app, main_async_loop)) 
    listener_thread.daemon = True
    listener_thread.start()
    print("Kafka Response Listener starting in separate thread.")

    app.add_handler(CommandHandler("start", start))
    app.add_handler(CommandHandler("help", help_command))
    app.add_handler(CommandHandler("weather", weather_command))
    app.add_handler(CommandHandler("forecast", forecast_command))

    app.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, handle_all_updates_to_kafka))
    app.add_handler(MessageHandler(filters.COMMAND, handle_all_updates_to_kafka))
    app.add_handler(CallbackQueryHandler(handle_all_updates_to_kafka))

    print("Telegram bot started and all messages sending to Kafka.")
    try:
        app.run_polling(poll_interval=1.0)
    except KeyboardInterrupt:
        print("Bot stopped by user.")
    finally:
        close_kafka_consumer()