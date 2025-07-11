# python_bot/main.py

import os
import threading
import asyncio
import logging
from telegram.ext import Application, CommandHandler, MessageHandler, filters

from .config import (
    TELEGRAM_BOT_TOKEN,
    KAFKA_BROKERS,
    KAFKA_COMMANDS_TOPIC,
    KAFKA_RESPONSES_TOPIC,
    NLP_SERVICE_URL,
    OPENWEATHER_API_KEY
)
from .kafka_service import init_kafka, kafka_response_listener, close_kafka_consumer, close_kafka_producer
from .nlp_client import close_nlp_client
from .handlers import (
    start_command, help_command, weather_command, forecast_command,
    mycity_command, subscribe_command, unsubscribe_command, track_command,
    untrack_command, mysubscriptions_command, mytracks_command,
    details_command, whattowear_command, ask_command
)
from .nlp_text_handler import process_nlp_text_message
from .commands import set_telegram_commands
from logger_config import setup_logging 
setup_logging(level=logging.INFO)
logger = logging.getLogger(__name__) 

async def post_init_setup(application: Application):
    """Callback function called after the bot application is initialized. Sets up Telegram bot commands."""
    await set_telegram_commands(application)
    logger.info("Telegram bot setup complete.")

async def post_shutdown(application: Application):
    """Callback function called when the bot application is shutting down. Ensures resources are closed cleanly."""
    logger.info("Application shutdown completed.")
    await close_kafka_producer() 
    await close_nlp_client()     

def run_kafka_listener_in_thread(app_instance: Application, loop_to_use: asyncio.AbstractEventLoop):
    """
    Target function for the Kafka response listener thread.
    Runs the Kafka consumer in a separate asyncio event loop.
    """
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    try:
        loop.run_until_complete(kafka_response_listener(loop_to_use))
    except asyncio.CancelledError:
        logger.info("Kafka listener thread cancelled.")
    except Exception as e:
        logger.error(f"Kafka listener thread encountered an error: {e}")
    finally:
        loop.close()
        logger.info("Kafka listener thread loop closed.")

def main() -> None:
    """Starts the Telegram bot and Kafka listener."""

    try:
        _ = TELEGRAM_BOT_TOKEN
        _ = KAFKA_BROKERS
        _ = KAFKA_COMMANDS_TOPIC
        _ = KAFKA_RESPONSES_TOPIC
        _ = NLP_SERVICE_URL
        _ = OPENWEATHER_API_KEY
    except EnvironmentError as e:
        logger.critical(e)
        exit(1)

    # Build the Telegram Application instance
    app = (
        Application.builder()
        .token(TELEGRAM_BOT_TOKEN)
        .post_init(post_init_setup) 
        .post_shutdown(post_shutdown) 
        .build()                    
    )

    app.bot_data["OPENWEATHER_API_KEY"] = OPENWEATHER_API_KEY
    
    # get the main event loop to pass to the Kafka listener thread
    main_async_loop = asyncio.get_event_loop()

    try:
        init_kafka(KAFKA_BROKERS, KAFKA_COMMANDS_TOPIC, KAFKA_RESPONSES_TOPIC, app)
    except Exception as e:
        logger.critical(f"Failed to initialize Kafka: {e}")
        exit(1)

    # start Kafka response listener in a separate daemon thread
    listener_thread = threading.Thread(target=run_kafka_listener_in_thread,
                                       args=(app, main_async_loop),
                                       daemon=True) # Daemon thread exits with main program
    listener_thread.start()
    logger.info("Kafka Response Listener starting in separate thread.")

    app.add_handler(CommandHandler("start", start_command))
    app.add_handler(CommandHandler("help", help_command))
    app.add_handler(CommandHandler("weather", weather_command))
    app.add_handler(CommandHandler("forecast", forecast_command))
    app.add_handler(CommandHandler("mycity", mycity_command))
    app.add_handler(CommandHandler("subscribe", subscribe_command))
    app.add_handler(CommandHandler("unsubscribe", unsubscribe_command))
    app.add_handler(CommandHandler("track", track_command))
    app.add_handler(CommandHandler("untrack", untrack_command))
    app.add_handler(CommandHandler("mysubscriptions", mysubscriptions_command))
    app.add_handler(CommandHandler("mytracks", mytracks_command))
    app.add_handler(CommandHandler("details", details_command))
    app.add_handler(CommandHandler("whattowear", whattowear_command))
    app.add_handler(CommandHandler("ask", ask_command))

    # Register a message handler for all non-command text messages to be processed by NLP
    app.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, process_nlp_text_message))

    logger.info("Telegram bot started polling...")
    try:
        # Start the bot's polling mechanism
        app.run_polling(poll_interval=5.0, stop_signals=None) 
    except KeyboardInterrupt:
        logger.info("Bot stopped by user via KeyboardInterrupt.")
    except Exception as e:
        logger.critical(f"Telegram bot encountered a critical error: {e}")
    finally:
        logger.info("Closing Kafka consumer and NLP HTTP client...")
        # Explicitly close Kafka consumer (producer is closed in post_shutdown)
        close_kafka_consumer()
        # Explicitly close NLP client (in case post_shutdown wasn't called)
        asyncio.run(close_nlp_client())
        logger.info("Application shut down cleanly.")

if __name__ == "__main__":
    main()