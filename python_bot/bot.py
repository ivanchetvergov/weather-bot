from telegram import Update
from telegram.ext import ApplicationBuilder, CommandHandler, MessageHandler, ContextTypes, filters
from weather_hand import get_weather

BOT_TOKEN = "8148245723:AAFuJrVVtirhVBdHcNbMavYu1uDMlJAtIzo"

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    name = update.effective_user.first_name
    await update.message.reply_text(f"Привет, {name}! Я твой погодный бот 🌦")

async def echo(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await update.message.reply_text(update.message.text)

async def weather_command(update: Update, context: ContextTypes.DEFAULT_TYPE):
    if context.args:
        city = " ".join(context.args)
        weather_info = await get_weather(city)
        await update.message.reply_text(weather_info)
    else:
        await update.message.reply_text("Укажи город, например:\n`/weather Москва`", parse_mode="Markdown")

if __name__ == "__main__":
    app = ApplicationBuilder().token(BOT_TOKEN).build()

    app.add_handler(CommandHandler("start", start))
    app.add_handler(CommandHandler("weather", weather_command))
    
    app.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, echo))

    print("bot is running...")
    app.run_polling()
