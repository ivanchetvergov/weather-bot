# python_bot/weather_api.py

import httpx
import os 

async def get_weather(city: str, api_key: str) -> str:

    url = "https://api.openweathermap.org/data/2.5/weather"
    params = {
        "q": city,
        "appid": api_key,
        "units": "metric",
        "lang": "ru"
    }

    try:
        async with httpx.AsyncClient() as client:
            response = await client.get(url, params=params, timeout=10.0)

        response.raise_for_status() # Вызовет исключение для 4xx/5xx ответов

        data = response.json()

        weather_desc = data["weather"][0]["description"].capitalize()
        temp = data["main"]["temp"]
        feels_like = data["main"]["feels_like"]
        humidity = data["main"]["humidity"]
        city_name = data.get("name", city) 

        return (
            f"Погода в городе {city_name}:\n"
            f"{weather_desc}\n"
            f"Температура: {temp}°C (ощущается как {feels_like}°C)\n"
            f"Влажность: {humidity}%"
        )
    except httpx.HTTPStatusError as e:
        if e.response.status_code == 404:
            return f"Город '{city}' не найден. Проверьте название."
        return f"Ошибка HTTP запроса: {e.response.status_code}"
    except httpx.RequestError as e:
        return f"Ошибка подключения к OpenWeatherMap: {e}"
    except (KeyError, IndexError) as e:
        print(f"Error parsing weather data for {city}: {e}, Raw data: {data}")
        return "Ошибка обработки данных о погоде. Попробуйте позже."