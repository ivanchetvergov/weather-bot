# nlp_service.py
import spacy
from spacy.matcher import Matcher
from typing import Dict, Any, Optional
from datetime import datetime, timedelta

from .nlp_patterns import NLP_PATTERNS, KNOWN_CITIES, CONDITION_KEYWORDS, DAY_MAPPINGS


class NlpService: 
    def __init__(self):
        try:
            self.nlp = spacy.load("ru_core_news_sm")
            print("SpaCy модель 'ru_core_news_sm' успешно загружена.")
        except OSError:
            print("Модель 'ru_core_news_sm' не найдена. Пожалуйста, запустите: python -m spacy download ru_core_news_sm")
            exit()

        self.matcher = Matcher(self.nlp.vocab)
        self._add_patterns()

        self.known_cities = KNOWN_CITIES
        self.known_cities_lower = [city.lower() for city in self.known_cities]

    def _add_patterns(self):
        for intent_name, patterns in NLP_PATTERNS.items():
            self.matcher.add(intent_name, patterns)
        print(f"Добавлено {len(NLP_PATTERNS)} групп паттернов.")

    def _extract_entities(self, doc, matched_label):
        entities = {}
        text_lower = doc.text.lower()

        found_city = None
        # 1. Попробуем найти город как SpaCy сущность (LOC/GPE)
        for ent in doc.ents:
            if (ent.label_ == "LOC" or ent.label_ == "GPE") and ent.text.lower() in self.known_cities:
                found_city = ent.text 
                break
        

        if not found_city:
            for city in KNOWN_CITIES: 
                if city.lower() in text_lower:
                    found_city = city 
                    break
        
        
        if found_city:
            entities["city"] = found_city

        # Извлечение даты (для прогноза)
        if matched_label == "GET_FORECAST":
            today = datetime.now().date()
            tomorrow = today + timedelta(days=1)
            day_after_tomorrow = today + timedelta(days=2)

            if "сегодня" in text_lower:
                entities["date"] = today.strftime("%Y-%m-%d")
            elif "завтра" in text_lower:
                entities["date"] = tomorrow.strftime("%Y-%m-%d")
            elif "послезавтра" in text_lower:
                entities["date"] = day_after_tomorrow.strftime("%Y-%m-%d")
            
            # Расширим логику для дней недели
            for day_name, day_index in DAY_MAPPINGS.items():
                if day_name in text_lower:
                    current_day_of_week = today.weekday() # 0 for Monday, 6 for Sunday
                    days_ahead = (day_index - current_day_of_week + 7) % 7
                    if days_ahead == 0 and "сегодня" not in text_lower: # Если день недели совпадает с текущим, но не "сегодня"
                        days_ahead = 7 # Значит, это следующий такой день недели
                    
                    target_date = today + timedelta(days=days_ahead)
                    entities["date"] = target_date.strftime("%Y-%m-%d")
                    break # Нашли день недели, выходим

        if matched_label in ["SUBSCRIBE", "MODIFY_SUBSCRIPTION"]:
            for keyword, condition_value in CONDITION_KEYWORDS.items():
                if keyword in text_lower:
                    entities["condition"] = condition_value
                    break
            # TODO: Добавить логику для "температура выше X", "ветер сильнее Y"
        return entities

    def process_text(self, text: str) -> Optional[Dict[str, Any]]: 
        doc = self.nlp(text)
        matches = self.matcher(doc)

        if matches:
            match_id, start, end = matches[0]
            matched_label = self.nlp.vocab.strings[match_id]

            intent = matched_label
            entities = self._extract_entities(doc, matched_label)
            
            return {
                "intent": intent,
                "entities": entities
            }
        else:
            return None


if __name__ == "__main__":
    nlp_service = NlpSevice() 

    test_phrases = [
        "Какая погода в Москве?",
        "Погода в Санкт-Петербурге",
        "Прогноз на завтра в Питере",
        "Прогноз погоды в Лондоне на послезавтра",
        "Прогноз на пятницу в спб", 
    ]

    for phrase in test_phrases:
        print(f"\nОбработка фразы: '{phrase}'")
        result = nlp_service.process_text(phrase)
        if result:
            print(f"  Распознано: {result.dumps(result, ensure_ascii=False, indent=2)}")
        else:
            print("  Команда не распознана.") 