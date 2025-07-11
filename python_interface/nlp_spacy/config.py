# nlp_spacy/config.py

import os

# path to the spacy model
SPACY_MODEL_NAME = os.getenv("spacy_model_name", "ru_core_news_sm")

# patterns for recognizing city names using spacy's entityruler
city_patterns = [
    {"label": "city", "pattern": [{"lower": "москва"}]},
    {"label": "city", "pattern": [{"lower": {"in": ["санкт-петербург", "спб", "питер"]}}]},
    {"label": "city", "pattern": [{"lower": "лондон"}]},
    {"label": "city", "pattern": [{"lower": "нью"}, {"orth": "-"}, {"lower": "йорк"}]}, # corrected for compound names
    {"label": "city", "pattern": [{"lower": "париж"}]},
    {"label": "city", "pattern": [{"lower": "берлин"}]},
    {"label": "city", "pattern": [{"lower": "рим"}]},
    {"label": "city", "pattern": [{"lower": "токио"}]},
    {"label": "city", "pattern": [{"lower": "пекин"}]},
    {"label": "city", "pattern": [{"lower": "сидней"}]},
    {"label": "city", "pattern": [{"lower": "дубай"}]},
    {"label": "city", "pattern": [{"lower": "стамбул"}]},
    {"label": "city", "pattern": [{"lower": "киев"}]},
    {"label": "city", "pattern": [{"lower": "минск"}]},
    {"label": "city", "pattern": [{"lower": "ташкент"}]},
    {"label": "city", "pattern": [{"lower": "алматы"}]},
    {"label": "city", "pattern": [{"lower": "сочи"}]},
    {"label": "city", "pattern": [{"lower": "казань"}]},
    {"label": "city", "pattern": [{"lower": "новосибирск"}]},
    {"label": "city", "pattern": [{"lower": "екатеринбург"}]},
    {"label": "city", "pattern": [{"lower": "краснодар"}]},
    {"label": "city", "pattern": [{"lower": "нижний"}, {"lower": "новгород"}]}, # corrected
    {"label": "city", "pattern": [{"lower": "самара"}]},
    {"label": "city", "pattern": [{"lower": "омск"}]},
    {"label": "city", "pattern": [{"lower": "челябинск"}]},
    {"label": "city", "pattern": [{"lower": "уфа"}]},
    {"label": "city", "pattern": [{"lower": "пермь"}]},
    {"label": "city", "pattern": [{"lower": "волгоград"}]},
    {"label": "city", "pattern": [{"lower": "ростов"}, {"orth": "-"}, {"lower": "на"}, {"orth": "-"}, {"lower": "дону"}]}, # corrected
    {"label": "city", "pattern": [{"lower": "воронеж"}]},
    {"label": "city", "pattern": [{"lower": "красноярск"}]},
    {"label": "city", "pattern": [{"lower": "владивосток"}]},
    {"label": "city", "pattern": [{"lower": "хабаровск"}]},
    {"label": "city", "pattern": [{"lower": "калининград"}]},
    {"label": "city", "pattern": [{"lower": "кемерово"}]},
    {"label": "city", "pattern": [{"lower": "иркутск"}]},
    {"label": "city", "pattern": [{"lower": "тюмень"}]},
    {"label": "city", "pattern": [{"lower": "саратов"}]},
    {"label": "city", "pattern": [{"lower": "тольятти"}]},
    {"label": "city", "pattern": [{"lower": "барнаул"}]},
    {"label": "city", "pattern": [{"lower": "ижевск"}]}
]

# dictionary for normalizing city names 
city_normalization = {
    "мск": "москва",
    "спб": "санкт-петербург",
    "питер": "санкт-петербург",
    "санктпетербург": "санкт-петербург",
    "нижний новгород": "нижний новгород"
}

# patterns for recognizing weather conditions using spacy's entityruler
weather_condition_patterns = [
    {"label": "weather_condition", "pattern": [{"lemma": "дождь"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "ливень"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "ветер"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "снег"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "град"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "солнце"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "облако"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "туман"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "температура"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "жара"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "холод"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "осадки"}]}, 
    {"label": "weather_condition", "pattern": [{"lower": "ясно"}]}, 
    {"label": "weather_condition", "pattern": [{"lower": "пасмурно"}]}, 
    {"label": "weather_condition", "pattern": [{"lower": {"in": ["сильный", "крепкий"]}}, {"lemma": "ветер"}]}, 
    {"label": "weather_condition", "pattern": [{"lower": {"in": ["проливной", "сильный"]}}, {"lemma": "дождь"}]}, 
    {"label": "weather_condition", "pattern": [{"lower": {"in": ["без", "отсутствие"]}}, {"lemma": "дождь"}]}, 
    {"label": "weather_condition", "pattern": [{"lower": "без"}, {"lemma": "осадки"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "похолодание"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "потепление"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "гололед"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "гололедица"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "шторм"}]}, 
    {"label": "weather_condition", "pattern": [{"lemma": "буря"}]} 
]

# patterns for recognizing dates using spacy's entityruler
date_patterns = [
    {"label": "date", "pattern": [{"lower": "сегодня"}]}, 
    {"label": "date", "pattern": [{"lower": "завтра"}]}, 
    {"label": "date", "pattern": [{"lower": "послезавтра"}]}, 
    {"label": "date", "pattern": [{"lower": "вчера"}]}, 
    {"label": "date", "pattern": [{"lower": "понедельник"}]}, 
    {"label": "date", "pattern": [{"lower": "вторник"}]}, 
    {"label": "date", "pattern": [{"lower": "среда"}]}, 
    {"label": "date", "pattern": [{"lower": "четверг"}]}, 
    {"label": "date", "pattern": [{"lower": "пятница"}]}, 
    {"label": "date", "pattern": [{"lower": "суббота"}]}, 
    {"label": "date", "pattern": [{"lower": "воскресенье"}]}, 
    {"label": "date", "pattern": [{"lower": "на"}, {"lower": {"in": ["завтра", "послезавтра", "сегодня", "днях", "этой", "следующей", "будущей"]}}, {"lower": "неделе", "op": "?"}]}, 
    {"label": "date", "pattern": [{"lower": "на"}, {"lower": {"in": ["понедельник", "вторник", "среда", "четверг", "пятница", "суббота", "воскресенье"]}}]}, 
    {"label": "date", "pattern": [{"lower": "через"}, {"like_num": true}, {"lower": {"in": ["день", "дня", "дней", "неделя", "недели", "недель"]}}]}, 
    {"label": "date", "pattern": [{"text": {"regex": "\\d{1,2}\\.\\d{1,2}"}}]}, 
    {"label": "date", "pattern": [{"text": {"regex": "\\d{1,2}\\.\\d{1,2}\\.\\d{4}"}}]} 
]

# dictionaries for normalization and internal use (not for matcher/ruler)
condition_normalization = {
    "дождь": "rain",
    "ливень": "heavy_rain",
    "ветер": "wind",
    "сильный ветер": "strong_wind",
    "температура": "temperature",
    "жара": "hot_temperature",
    "холод": "cold_temperature",
    "осадки": "precipitation",
    "солнце": "clear_sky",
    "ясно": "clear_sky",
    "снег": "snow",
    "метель": "blizzard",
    "пасмурно": "cloudy",
    "туман": "fog",
    "без дождя": "no_rain",
    "без осадков": "no_precipitation", # added for example
    "похолодание": "cooling",
    "потепление": "warming",
    "гололед": "ice",
    "гололедица": "ice",
    "шторм": "storm",
    "буря": "storm"
}

# mapping of weekdays to their numerical index (monday=0, sunday=6)
day_mappings_num = {
    "понедельник": 0, 
    "вторник": 1, 
    "среда": 2, 
    "четверг": 3, 
    "пятница": 4, 
    "суббота": 5, 
    "воскресенье": 6 
}