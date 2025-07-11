# python_bot/config.py

import os
from dotenv import load_dotenv

# load environment variables from .env file
load_dotenv()

# --- telegram bot api token ---
TELEGRAM_BOT_TOKEN = os.getenv("TELEGRAM_BOT_TOKEN")

# --- kafka configuration ---
KAFKA_BROKERS = os.getenv("KAFKA_BROKERS")
# add default values if environment variables are not set
KAFKA_COMMANDS_TOPIC = os.getenv("KAFKA_COMMANDS_TOPIC", "bot_commands")
KAFKA_RESPONSES_TOPIC = os.getenv("KAFKA_RESPONSES_TOPIC", "bot_responses")

# --- nlp service configuration ---
# url of your separately running nlp service (e.g., nlp_spacy)
NLP_SERVICE_URL = os.getenv("NLP_SERVICE_URL", "http://localhost:8000/process_text")

# --- openweathermap api key ---
OPENWEATHER_API_KEY = os.getenv("OPENWEATHER_API_KEY")

# --- check for required environment variables ---
# if any of these variables are missing, the application will not start.
REQUIRED_ENV_VARS = [
    "TELEGRAM_BOT_TOKEN",
    "KAFKA_BROKERS",
    "KAFKA_COMMANDS_TOPIC",
    "KAFKA_RESPONSES_TOPIC",
    "NLP_SERVICE_URL",
    "OPENWEATHER_API_KEY" # check that it exists if it's needed in bot_data
]

for var_name in REQUIRED_ENV_VARS:
    if os.getenv(var_name) is None:
        raise EnvironmentError(f"missing required environment variable: {var_name}")