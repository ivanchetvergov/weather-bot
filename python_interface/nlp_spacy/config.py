import os

# path to the spacy model
SPACY_MODEL_NAME = os.getenv("spacy_model_name", "ru_core_news_sm")

# patterns for recognizing CITY names using spacy's entityruler
CITY_PATTERNS = [
    {"label": "CITY", "pattern": [{"lemma": "москва"}]},
    {"label": "CITY", "pattern": [{"lemma": {"in": ["санкт-петербург", "спб", "питер"]}}]},
    {"label": "CITY", "pattern": [{"lemma": "лондон"}]},
    {"label": "CITY", "pattern": [{"lemma": "нью"}, {"orth": "-"}, {"lemma": "йорк"}]}, # corrected for compound names
    {"label": "CITY", "pattern": [{"lemma": "париж"}]},
    {"label": "CITY", "pattern": [{"lemma": "берлин"}]},
    {"label": "CITY", "pattern": [{"lemma": "рим"}]},
    {"label": "CITY", "pattern": [{"lemma": "токио"}]},
    {"label": "CITY", "pattern": [{"lemma": "пекин"}]},
    {"label": "CITY", "pattern": [{"lemma": "сидней"}]},
    {"label": "CITY", "pattern": [{"lemma": "дубай"}]},
    {"label": "CITY", "pattern": [{"lemma": "стамбул"}]},
    {"label": "CITY", "pattern": [{"lemma": "киев"}]},
    {"label": "CITY", "pattern": [{"lemma": "минск"}]},
    {"label": "CITY", "pattern": [{"lemma": "ташкент"}]},
    {"label": "CITY", "pattern": [{"lemma": "алматы"}]},
    {"label": "CITY", "pattern": [{"lemma": "сочи"}]},
    {"label": "CITY", "pattern": [{"lemma": "казань"}]},
    {"label": "CITY", "pattern": [{"lemma": "новосибирск"}]},
    {"label": "CITY", "pattern": [{"lemma": "екатеринбург"}]},
    {"label": "CITY", "pattern": [{"lemma": "краснодар"}]},
    {"label": "CITY", "pattern": [{"lemma": "нижний"}, {"lemma": "новгород"}]}, # corrected
    {"label": "CITY", "pattern": [{"lemma": "самара"}]},
    {"label": "CITY", "pattern": [{"lemma": "омск"}]},
    {"label": "CITY", "pattern": [{"lemma": "челябинск"}]},
    {"label": "CITY", "pattern": [{"lemma": "уфа"}]},
    {"label": "CITY", "pattern": [{"lemma": "пермь"}]},
    {"label": "CITY", "pattern": [{"lemma": "волгоград"}]},
    {"label": "CITY", "pattern": [{"lemma": "ростов"}, {"orth": "-"}, {"lemma": "на"}, {"orth": "-"}, {"lemma": "дону"}]}, # corrected
    {"label": "CITY", "pattern": [{"lemma": "воронеж"}]},
    {"label": "CITY", "pattern": [{"lemma": "красноярск"}]},
    {"label": "CITY", "pattern": [{"lemma": "владивосток"}]},
    {"label": "CITY", "pattern": [{"lemma": "хабаровск"}]},
    {"label": "CITY", "pattern": [{"lemma": "калининград"}]},
    {"label": "CITY", "pattern": [{"lemma": "кемерово"}]},
    {"label": "CITY", "pattern": [{"lemma": "иркутск"}]},
    {"label": "CITY", "pattern": [{"lemma": "тюмень"}]},
    {"label": "CITY", "pattern": [{"lemma": "саратов"}]},
    {"label": "CITY", "pattern": [{"lemma": "тольятти"}]},
    {"label": "CITY", "pattern": [{"lemma": "барнаул"}]},
    {"label": "CITY", "pattern": [{"lemma": "ижевск"}]}
]

# dictionary for normalizing CITY names 
CITY_NORMALIZATION = { 
    "мск": "москва",
    "спб": "санкт-петербург",
    "питер": "санкт-петербург",
    "санктпетербург": "санкт-петербург",
    "нижний новгород": "нижний новгород"
}

# patterns for recognizing weather conditions using spacy's entityruler
WEATHER_CONDITION_PATTERNS = [ 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "дождь"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "ливень"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "ветер"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "снег"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "град"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "солнце"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "облако"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "туман"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "температура"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "жара"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "холод"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "осадки"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lower": "ясно"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lower": "пасмурно"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lower": {"in": ["сильный", "крепкий"]}}, {"lemma": "ветер"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lower": {"in": ["проливной", "сильный"]}}, {"lemma": "дождь"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lower": {"in": ["без", "отсутствие"]}}, {"lemma": "дождь"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lower": "без"}, {"lemma": "осадки"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "похолодание"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "потепление"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "гололед"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "гололедица"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "шторм"}]}, 
    {"label": "WEATHER_CONDITION", "pattern": [{"lemma": "буря"}]} 
]

# patterns for recognizing dates using spacy's entityruler
DATE_PATTERNS = [ 
    {"label": "DATE", "pattern": [{"lower": "сегодня"}]}, 
    {"label": "DATE", "pattern": [{"lower": "завтра"}]}, 
    {"label": "DATE", "pattern": [{"lower": "послезавтра"}]}, 
    {"label": "DATE", "pattern": [{"lower": "вчера"}]}, 
    {"label": "DATE", "pattern": [{"lower": "понедельник"}]}, 
    {"label": "DATE", "pattern": [{"lower": "вторник"}]}, 
    {"label": "DATE", "pattern": [{"lower": "среда"}]}, 
    {"label": "DATE", "pattern": [{"lower": "четверг"}]}, 
    {"label": "DATE", "pattern": [{"lower": "пятница"}]}, 
    {"label": "DATE", "pattern": [{"lower": "суббота"}]}, 
    {"label": "DATE", "pattern": [{"lower": "воскресенье"}]}, 
    {"label": "DATE", "pattern": [{"lower": "на"}, {"lower": {"in": ["завтра", "послезавтра", "сегодня", "днях", "этой", "следующей", "будущей"]}}, {"lower": "неделе", "op": "?"}]}, 
    {"label": "DATE", "pattern": [{"lower": "на"}, {"lower": {"in": ["понедельник", "вторник", "среда", "четверг", "пятница", "суббота", "воскресенье"]}}]}, 
    {"label": "DATE", "pattern": [{"lower": "через"}, {"like_num": True}, {"lower": {"in": ["день", "дня", "дней", "неделя", "недели", "недель"]}}]}, 
    {"label": "DATE", "pattern": [{"text": {"regex": "\\d{1,2}\\.\\d{1,2}"}}]}, 
    {"label": "DATE", "pattern": [{"text": {"regex": "\\d{1,2}\\.\\d{1,2}\\.\\d{4}"}}]} 
]

# dictionaries for normalization and internal use (not for matcher/ruler)
CONDITION_NORMALIZATION = { # Renamed to uppercase
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
DAY_MAPPINGS_NUM = { # Renamed to uppercase
    "понедельник": 0, 
    "вторник": 1, 
    "среда": 2, 
    "четверг": 3, 
    "пятница": 4, 
    "суббота": 5, 
    "воскресенье": 6 
}