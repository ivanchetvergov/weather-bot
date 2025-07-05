# telegram_bot/bot.py

import os
import json
from dotenv import load_dotenv
from telegram import Update
from telegram.ext import ApplicationBuilder, CommandHandler, MessageHandler, ContextTypes, filters, CallbackQueryHandler
from confluent_kafka import Producer, Consumer, KafkaException, KafkaError # Убедись, что KafkaError здесь!
import threading
import asyncio
import time # Для time.time()

# --- 1. Загрузка переменных окружения ---
load_dotenv() 

BOT_TOKEN = os.getenv("TELEGRAM_BOT_TOKEN")
KAFKA_BROKERS = os.getenv("KAFKA_BROKERS")
KAFKA_COMMANDS_TOPIC = os.getenv("KAFKA_COMMANDS_TOPIC")
KAFKA_RESPONSES_TOPIC = os.getenv("KAFKA_RESPONSES_TOPIC")

if not all([BOT_TOKEN, KAFKA_BROKERS, KAFKA_COMMANDS_TOPIC, KAFKA_RESPONSES_TOPIC]):
    print("error. not all var loaded.")
    exit(1) 

# --- 2. Инициализация Kafka Producer ---
try:
    kafka_producer = Producer({'bootstrap.servers': KAFKA_BROKERS})
    print(f"Kafka Producer connected to {KAFKA_BROKERS}")
except Exception as e:
    print(f"err while connecting Kafka Producer: {e}")
    exit(1)

# --- 3. Инициализация Kafka Consumer для ответов от бэкенда ---
kafka_consumer_config = {
    'bootstrap.servers': KAFKA_BROKERS,
    'group.id': 'telegram_bot_response_group', 
    'auto.offset.reset': 'earliest' 
}
try:
    kafka_consumer = Consumer(kafka_consumer_config)
    kafka_consumer.subscribe([KAFKA_RESPONSES_TOPIC])
    print(f"Kafka Consumer subscribed to topic '{KAFKA_RESPONSES_TOPIC}'")
except Exception as e:
    print(f"err while connecting Kafka Consumer: {e}")
    exit(1)

# --- 4. Функция для отправки Telegram Update в Kafka ---
def send_telegram_update_to_kafka(update_obj: Update):
    """
    Формирует и отправляет полный объект Telegram Update в Kafka топик.
    Использует user_id в качестве ключа для сохранения порядка сообщений от одного пользователя.
    """
    user_id = None
    event_type = "unknown"
    kafka_key = "" # Ключ для Kafka сообщения

    # Определяем user_id и тип события
    if update_obj.message:
        user_id = update_obj.message.from_user.id
        event_type = "telegram_message"
        kafka_key = str(user_id) 
    elif update_obj.callback_query:
        user_id = update_obj.callback_query.from_user.id
        event_type = "telegram_callback_query"
        kafka_key = str(user_id) 

    if user_id is None:
        print(f"Предупреждение: Не удалось определить user_id для update. Пропускаем отправку в Kafka.")
        return

    # Получаем timestamp сообщения. Используем время сообщения, если доступно, иначе текущее.
    message_timestamp = None
    if update_obj.effective_message and update_obj.effective_message.date:
        message_timestamp = int(update_obj.effective_message.date.timestamp())
    elif update_obj.callback_query and update_obj.callback_query.message and update_obj.callback_query.message.date:
        message_timestamp = int(update_obj.callback_query.message.date.timestamp())
    else:
        message_timestamp = int(time.time()) # Используй time.time() из стандартной библиотеки

    # Формируем полную структуру сообщения для Kafka
    kafka_payload = {
        "event_type": event_type,
        "timestamp": message_timestamp,
        "source": "telegram_bot",
        "data": update_obj.to_dict() # telebot.Update.to_dict() сериализует весь объект
    }

    try:
        kafka_producer.produce(
            KAFKA_COMMANDS_TOPIC,
            key=kafka_key.encode('utf-8'), 
            value=json.dumps(kafka_payload).encode('utf-8'), 
            callback=delivery_report 
        )
        kafka_producer.poll(0) # <-- ВАЖНО: Добавлено для отправки буферизованных сообщений
        print(f"Отправлено обновление от пользователя {user_id} (тип: {event_type}) в Kafka топик '{KAFKA_COMMANDS_TOPIC}'")
    except Exception as e:
        print(f"Ошибка при отправке сообщения в Kafka: {e}")

def delivery_report(err, msg):
    """Колбэк для отчета о доставке Kafka."""
    if err is not None:
        print(f"Ошибка доставки сообщения Kafka: {err}")
    else:
        # print(f"Сообщение доставлено в {msg.topic()} [{msg.partition()}] @ {msg.offset()}")
        pass # Не печатаем каждый раз, чтобы не спамить консоль


# --- 5. Асинхронный слушатель ответов из Kafka (работает в отдельном потоке) ---
async def kafka_response_listener(application: ApplicationBuilder):
    """
    Слушает топик с ответами от бэкенда и отправляет их пользователям.
    """
    while True:
        try:
            msg = kafka_consumer.poll(timeout=1.0) # Ждем сообщение 1 секунду
            if msg is None:
                await asyncio.sleep(0.1) # Немного ждем, если нет сообщений
                continue
            if msg.error():
                # Правильная проверка на конец раздела
                if msg.error().code() == KafkaError._PARTITION_EOF: # <-- ИСПОЛЬЗУЙ KafkaError!
                    # Конец раздела, ничего страшного, продолжаем
                    continue
                else:
                    print(f"Ошибка Kafka Consumer: {msg.error()}")
                    continue

            # Парсим JSON из сообщения
            payload = msg.value().decode('utf-8')
            response_data = json.loads(payload)

            telegram_user_id = response_data.get('telegram_user_id')
            message_text = response_data.get('message')
            parse_mode = response_data.get('parse_mode', None) # Может быть 'HTML' или 'MarkdownV2'

            if telegram_user_id and message_text:
                print(f"Получен ответ из Kafka для пользователя {telegram_user_id}: {message_text[:50]}...")
                # Отправляем сообщение пользователю через Telegram API
                try:
                    await application.bot.send_message(
                        chat_id=telegram_user_id,
                        text=message_text,
                        parse_mode=parse_mode # Используем parse_mode, если указан
                    )
                except Exception as e:
                    print(f"Ошибка при отправке сообщения Telegram API пользователю {telegram_user_id}: {e}")
            else:
                print(f"Некорректный формат ответа из Kafka: {response_data}")

        except json.JSONDecodeError as e:
            print(f"Ошибка парсинга JSON из Kafka: {e} | Сообщение: {payload}")
        except Exception as e:
            print(f"Непредвиденная ошибка в слушателе Kafka: {e}")
        finally:
            kafka_consumer.commit(asynchronous=True) # Коммитим смещение асинхронно

# --- 6. Основные обработчики Telegram бота ---

# Все текстовые сообщения и колбэки перехватываем и отправляем в Kafka
async def handle_all_updates_to_kafka(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    # Здесь мы отправляем ВЕСЬ объект Update в Kafka
    send_telegram_update_to_kafka(update)
    # Можно отправить мгновенный ответ пользователю, чтобы он знал, что запрос принят
    if update.message:
        await update.message.reply_text("✅ Ваш запрос принят и обрабатывается...")
    elif update.callback_query:
        await update.callback_query.answer("Запрос принят ✅") # Закрываем уведомление о колбэке

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    # Для команды /start тоже отправляем в Kafka, чтобы бэкенд мог зарегистрировать пользователя
    send_telegram_update_to_kafka(update)
    # Немедленный ответ от бота (не дожидаясь бэкенда)
    name = update.effective_user.first_name if update.effective_user else "друг"
    await update.message.reply_text(f"Привет, {name}! Я твой погодный бот 🌦. Ваш запрос на старт отправлен.")


# --- 7. Запуск бота ---
if __name__ == "__main__":
    app = ApplicationBuilder().token(BOT_TOKEN).build()

    # Запускаем Kafka Response Listener в отдельном потоке
    def run_kafka_listener_in_thread(app_instance):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        # Убедимся, что луп не блокируется, чтобы он мог обрабатывать сигналы завершения
        try:
            loop.run_until_complete(kafka_response_listener(app_instance))
        except asyncio.CancelledError:
            pass # Это нормально при отмене задачи
        finally:
            loop.close()

    listener_thread = threading.Thread(target=run_kafka_listener_in_thread, args=(app,))
    listener_thread.daemon = True # Поток завершится при завершении основной программы
    listener_thread.start()
    print("Kafka Response Listener запущен в отдельном потоке.")

    # Добавляем обработчики Telegram
    app.add_handler(CommandHandler("start", start)) # /start тоже идет через Kafka
    app.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, handle_all_updates_to_kafka))
    app.add_handler(MessageHandler(filters.COMMAND, handle_all_updates_to_kafka)) # Все команды идут в Kafka
    app.add_handler(CallbackQueryHandler(handle_all_updates_to_kafka)) # Все колбэки идут в Kafka

    print("Telegram bot запущен и отправляет все обновления в Kafka.")
    try:
        app.run_polling(poll_interval=1.0) # Интервал опроса Telegram API
    except KeyboardInterrupt:
        print("Бот остановлен пользователем.")
    finally:
        # Важно: При остановке polling, поток-слушатель Kafka Consumer должен завершиться.
        # Поскольку он daemon=True, он завершится вместе с основной программой.
        # Однако, для более чистого завершения, можно добавить флаг или сигнал.
        # Для простоты текущей реализации, завершение консьюмера здесь будет работать.
        kafka_consumer.close() # Закрываем потребителя при завершении
        print("Kafka Consumer закрыт.")