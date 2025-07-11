# python_bot/nlp_patterns.py

# --- общие токены для переиспользования ---
PREP_IN_CITY = {"LOWER": {"IN": ["в", "для", "по"]}, "OP": "?"} # Добавлен "по"
CITY = {"ENT_TYPE": "CITY", "OP": "?"} # OP: "?" означает, что город может отсутствовать
DATE = {"ENT_TYPE": "DATE"}
WEATHER = {"LEMMA": "погода"}
WEATHER_COND = {"ENT_TYPE": "WEATHER_CONDITION"}
TEMPERATURE = {"LEMMA": "температура"}
WIND = {"LEMMA": "ветер"}
TEMP_MODIFIER = {"LOWER": {"IN": ["выше", "больше", "ниже", "меньше", "около"]}} # Добавлен TEMP_MODIFIER
TIME_EXPRESSION = {"ENT_TYPE": "TIME"} # Добавлен TIME_EXPRESSION, если SpaCy будет его распознавать

# --- spaCy matcher patterns by intent ---
NLP_PATTERNS = {
    "/weather": [
        # Основные запросы погоды
        [WEATHER, PREP_IN_CITY, CITY, {"LOWER": "сейчас", "OP": "?"}],
        [{"LEMMA": "какой"}, WEATHER, PREP_IN_CITY, CITY],
        [{"LEMMA": {"IN": ["что", "как"]}}, {"LOWER": "там", "OP": "?"}, {"LEMMA": "погода"}, PREP_IN_CITY, CITY],
        [{"LEMMA": "показывать"}, WEATHER, PREP_IN_CITY, CITY],
        [WEATHER, CITY],
        [CITY, WEATHER], # "Москва погода"

        # Запросы с датой (сегодня/завтра/послезавтра) - универсализированный паттерн
        [WEATHER, PREP_IN_CITY, CITY, {"LOWER": "на", "OP": "?"}, {"LOWER": {"IN": ["сегодня", "завтра", "послезавтра"]}}],
        [{"LOWER": {"IN": ["сегодня", "завтра", "послезавтра"]}}, WEATHER, PREP_IN_CITY, CITY], # "Завтра погода в Питере"

        # Запросы температуры
        [TEMPERATURE, PREP_IN_CITY, CITY],
        [{"LEMMA": {"IN": ["сколько", "какая"]}}, {"LEMMA": "температура"}, PREP_IN_CITY, CITY],
        [CITY, {"LEMMA": "температура"}], # "Москва температура"

        # Запросы ветра
        [{"LEMMA": "какой"}, WIND, PREP_IN_CITY, CITY],
        [WIND, PREP_IN_CITY, CITY], # "ветер в Москве"
        [{"LEMMA": "сильный", "OP": "?"}, WIND, PREP_IN_CITY, CITY] # "сильный ветер в Москве"
    ],

    "/mycity": [
        [{"LEMMA": {"IN": ["установить", "поставить", "сделать", "изменить", "сменить"]}}, 
        {"LOWER": {"IN": ["мой", "свой"]}, "OP": "?"}, {"LEMMA": "город"}, 
        {"LOWER": {"IN": ["по умолчанию", "по-умолчанию", "дефолтным"]}, "OP": "?"}, CITY],

        [{"LEMMA": {"IN": ["установить", "сделать", "поставить"]}}, 
        CITY, {"LOWER": {"IN": ["по умолчанию", "по-умолчанию", "дефолтным"]}, "OP": "?"}], # "установить Кемерово по умолчанию"

        [{"LOWER": {"IN": ["мой", "город"]}}, CITY], # "Мой город Кемерово"
        [CITY, {"LOWER": {"IN": ["мой", "город"]}}], # "Кемерово мой город"

        [{"LEMMA": {"IN": ["показать", "какой"]}}, {"LOWER": "мой"}, {"LEMMA": "город"}] # "Покажи мой город", "Какой мой город"
    ],

    "/forecast": [
        # Общие запросы прогноза
        [{"LEMMA": "прогноз"}, {"LOWER": "на", "OP": "?"}, DATE, PREP_IN_CITY, CITY, {"LEMMA": "погода", "OP": "?"}],
        [{"LEMMA": "прогноз"}, WEATHER, PREP_IN_CITY, CITY, {"LOWER": "на", "OP": "?"}, DATE],
        [{"LEMMA": "какой"}, {"LEMMA": "прогноз"}, {"LOWER": "на", "OP": "?"}, DATE, PREP_IN_CITY, CITY],
        [{"LEMMA": "прогноз"}, PREP_IN_CITY, CITY], # "прогноз в Москве" (на сегодня по умолчанию)
        [{"LEMMA": "прогноз"}, DATE], # "прогноз на завтра" (для города по умолчанию)

        # Запросы о конкретных условиях на дату
        [{"LEMMA": "быть"}, {"LOWER": "ли"}, WEATHER_COND, PREP_IN_CITY, CITY, {"LOWER": "на", "OP": "?"}, DATE], # "Будет ли дождь в Москве завтра?"
        [{"LEMMA": "быть"}, {"LOWER": "ли"}, WEATHER_COND, {"LOWER": "на", "OP": "?"}, DATE], # "Будет ли дождь завтра?"

        # В обратном порядке
        [DATE, PREP_IN_CITY, CITY, {"LEMMA": "прогноз"}, {"LEMMA": "погода", "OP": "?"}]
    ],

    "/subscribe": [
        # Подписка на общее условие/погоду
        [{"LEMMA": {"IN": ["подписать", "подписаться"]}}, {"LOWER": "меня", "OP": "?"}, {"LOWER": "на"}, WEATHER_COND, PREP_IN_CITY, CITY],
        [{"LEMMA": {"IN": ["подписать", "подписаться"]}}, {"LOWER": "меня", "OP": "?"}, {"LOWER": "на"}, WEATHER, PREP_IN_CITY, CITY], # Для "подписаться на погоду в Москве"
        [{"LEMMA": {"IN": ["хотеть", "нужно"]}}, {"LEMMA": "уведомление"}, {"LOWER": {"IN": ["о", "про", "на"]}, "OP": "?"}, WEATHER_COND, PREP_IN_CITY, CITY],
        [{"LEMMA": {"IN": ["создавать", "добавить"]}}, {"LEMMA": "подписка"}, PREP_IN_CITY, CITY, {"LOWER": "на"}, WEATHER_COND],

        # Подписка по температуре
        [{"LEMMA": {"IN": ["подписать", "подписаться"]}}, {"LOWER": "на", "OP": "?"}, CITY, TEMPERATURE, TEMP_MODIFIER, {"LIKE_NUM": True, "TAG": {"IN": ["NUM", "CD"]}}],
        [{"LEMMA": {"IN": ["подписать", "подписаться"]}}, {"LOWER": "на"}, TEMPERATURE, TEMP_MODIFIER, {"LIKE_NUM": True, "TAG": {"IN": ["NUM", "CD"]}}, PREP_IN_CITY, CITY],

        # Подписка по ветру
        [{"LEMMA": {"IN": ["подписать", "подписаться"]}}, {"LOWER": "на", "OP": "?"}, CITY, WIND, TEMP_MODIFIER, {"LIKE_NUM": True, "TAG": {"IN": ["NUM", "CD"]}}],
        [{"LEMMA": {"IN": ["подписать", "подписаться"]}}, {"LOWER": "на"}, WIND, TEMP_MODIFIER, {"LIKE_NUM": True, "TAG": {"IN": ["NUM", "CD"]}}, PREP_IN_CITY, CITY],

        # Подписка на уведомления о дожде/солнце
        [{"LEMMA": {"IN": ["уведомлять", "сообщить"]}}, {"LOWER": "меня", "OP": "?"}, {"LOWER": {"IN": ["о", "про"]}, "OP": "?"}, WEATHER_COND, PREP_IN_CITY, CITY],
        [{"LEMMA": "уведомление"}, {"LOWER": "о"}, WEATHER_COND, PREP_IN_CITY, CITY],

        # Уточнение времени уведомления
        [{"LEMMA": "уведомлять"}, {"LOWER": "меня", "OP": "?"}, {"LOWER": "в"}, TIME_EXPRESSION, PREP_IN_CITY, CITY]
    ],

    "/unsubscribe": [
        [{"LEMMA": {"IN": ["отписать", "отписаться"]}}, {"LOWER": "меня", "OP": "?"}, {"LOWER": "от"}, WEATHER_COND, PREP_IN_CITY, CITY],
        [{"LEMMA": {"IN": ["отписать", "отписаться"]}}, {"LOWER": "меня", "OP": "?"}, {"LOWER": "от"}, WEATHER, PREP_IN_CITY, CITY],
        [{"LEMMA": {"IN": ["отменить", "удалить", "убрать"]}}, {"LEMMA": "подписка"}, {"LOWER": "на", "OP": "?"}, CITY],
        [{"LEMMA": {"IN": ["отменить", "удалить", "убрать"]}}, {"LEMMA": "уведомление"}, {"LOWER": {"IN": ["о", "про"]}, "OP": "?"}, WEATHER_COND, PREP_IN_CITY, CITY],

        # Отмена по температуре/ветру
        [{"LEMMA": {"IN": ["отменить", "удалить"]}}, {"LEMMA": "подписка"}, {"LOWER": "на", "OP": "?"}, CITY, TEMPERATURE, TEMP_MODIFIER, {"LIKE_NUM": True, "TAG": {"IN": ["NUM", "CD"]}}],
        [{"LEMMA": {"IN": ["отменить", "удалить"]}}, {"LEMMA": "подписка"}, {"LOWER": "на", "OP": "?"}, CITY, WIND, TEMP_MODIFIER, {"LIKE_NUM": True, "TAG": {"IN": ["NUM", "CD"]}}],
    ]
}