import httpx
import os
from dotenv import load_dotenv

load_dotenv() 

OPENWEATHER_API_KEY = os.getenv("OPENWEATHER_API_KEY")

async def get_weather(city: str) -> str:
    url = "https://api.openweathermap.org/data/2.5/weather"
    params = {
        "q": city,
        "appid": OPENWEATHER_API_KEY,
        "units": "metric",
        "lang": "ru"
    }

    async with httpx.AsyncClient() as client:
        response = await client.get(url, params=params)

    if response.status_code != 200:
        return "❌ Не удалось получить данные о погоде. Проверь название города."

    data = response.json()

    try:
        weather_desc = data["weather"][0]["description"].capitalize()
        temp = data["main"]["temp"]
        feels_like = data["main"]["feels_like"]
        humidity = data["main"]["humidity"]
        city_name = data.get("name", city)
    except (KeyError, IndexError):
        return "❌ Ошибка обработки данных. Ответ от API был неожиданным."

    return (
        f"📍 Погода в городе {city_name}:\n"
        f"{weather_desc}\n"
        f"🌡 Температура: {temp}°C (ощущается как {feels_like}°C)\n"
        f"💧 Влажность: {humidity}%"
    )
