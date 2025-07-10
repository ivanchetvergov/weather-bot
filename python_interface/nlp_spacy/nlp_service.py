# nlp_service.py

import spacy
from spacy.matcher import Matcher
from spacy.pipeline import EntityRuler
from typing import Dict, Any, Optional
from datetime import datetime, timedelta
import json
import re   
import sys

from nlp_patterns import NLP_PATTERNS, KNOWN_CITIES, CONDITION_KEYWORDS, DAY_MAPPINGS

class NlpService: 
    def __init__(self):
        try:
            self.nlp = spacy.load("ru_core_news_sm")
            print("SpaCy модель 'ru_core_news_sm' успешно загружена.")
        except OSError:
            print("Модель 'ru_core_news_sm' не найдена. Пожалуйста, запустите: python -m spacy download ru_core_news_sm")
            sys.exit("SpaCy model 'ru_core_news_sm' not found. Please run: python -m spacy download ru_core_news_sm")

        if "entity_ruler" not in self.nlp.pipe_names: 
            self.ruler = self.nlp.add_pipe("entity_ruler", before="ner")
            
            city_patterns = []
            for city_name in KNOWN_CITIES:
                city_patterns.append({"label": "CITY", "pattern": [{"LOWER": city_name}]})
                
                # Добавляем паттерны для многословных городов (например, "Санкт-Петербург")
                if " " in city_name or "-" in city_name:
                    tokens = [token.lower() for token in re.split(r'[\s-]', city_name) if token]
                    if tokens:
                        city_patterns.append({"label": "CITY", "pattern": [{"LOWER": token} for token in tokens]})

            self.ruler.add_patterns(city_patterns)
            print(f"Добавлено {len(city_patterns)} паттернов городов в EntityRuler.")
        else:
            print("EntityRuler уже добавлен в пайплайн SpaCy.")

        self.matcher = Matcher(self.nlp.vocab)
        self._add_patterns()

        self.known_cities_lower = set(KNOWN_CITIES) 

    def _add_patterns(self):
        # Теперь ключи в NLP_PATTERNS - это уже команды (/weather, /forecast и т.д.)
        for command_name, patterns in NLP_PATTERNS.items():
            self.matcher.add(command_name, patterns)
        print(f"Добавлено {len(NLP_PATTERNS)} групп паттернов.")

    def _extract_entities(self, doc, matched_command): # matched_command теперь /weather, /forecast и т.д.
        entities = {}
        text_lower = doc.text.lower()
        found_city = None

        if matched_command in ["/weather", "/mycity", "/forecast", "/subscribe", \
                               "/unsubscribe", "/modify_subscription"]:
            # Извлечение города из Doc.ents (более надежно)
            for ent in doc.ents:    
                if (ent.label_ == "CITY" or ent.label_ == "LOC" or ent.label_ == "GPE") \
                and ent.lemma_.lower() in self.known_cities_lower: # Проверяем на принадлежность к нашим городам
                    found_city = ent.lemma_.lower() 
                    break

            # Если не нашли через сущности SpaCy, пытаемся найти по ключевым словам в лемматизированном тексте
            if not found_city:
                lemmatized_text = " ".join([token.lemma_.lower() for token in doc])
                for city in self.known_cities_lower: 
                    if city in lemmatized_text:
                        found_city = city
                        break
            
            if found_city:
                entities["city"] = found_city

        # Извлечение даты для прогноза
        if matched_command == "/forecast":
            today = datetime.now().date()
            
            if "сегодня" in text_lower:
                entities["date"] = today.strftime("%Y-%m-%d")
            elif "завтра" in text_lower:
                entities["date"] = (today + timedelta(days=1)).strftime("%Y-%m-%d")
            elif "послезавтра" in text_lower:
                entities["date"] = (today + timedelta(days=2)).strftime("%Y-%m-%d")
            
            for day_name, day_index in DAY_MAPPINGS.items():
                if day_name in text_lower:
                    current_day_of_week = today.weekday() 
                    days_ahead = (day_index - current_day_of_week + 7) % 7
                    # Если день недели совпадает с текущим, но пользователь не сказал "сегодня",
                    # предполагаем, что речь идёт о следующей неделе.
                    if days_ahead == 0 and "сегодня" not in text_lower: 
                        days_ahead = 7 
                    
                    target_date = today + timedelta(days=days_ahead)
                    entities["date"] = target_date.strftime("%Y-%m-%d")
                    break 
        
        # Извлечение условий и параметров для подписок
        if matched_command in ["/subscribe", "/modify_subscription"]:
            for keyword, condition_value in CONDITION_KEYWORDS.items():
                if keyword in text_lower:
                    entities["condition"] = condition_value
                    break
            
            # Извлечение "температура выше X"
            temp_above_match = re.search(r'(?:температура|теплее)\s*(?:выше|больше)\s*(\d+)', text_lower)
            if temp_above_match:
                try:
                    entities["temp_above"] = float(temp_above_match.group(1))
                except ValueError:
                    pass

            # Извлечение "ветер сильнее Y"
            wind_speed_match = re.search(r'(?:ветер)\s*(?:сильнее|быстрее)\s*(\d+\.?\d*)', text_lower)
            if wind_speed_match:
                try:
                    entities["wind_speed_gt"] = float(wind_speed_match.group(1))
                except ValueError:
                    pass
            
            # Извлечение "дождь" / "без дождя"
            if "дождь" in text_lower and "без" not in text_lower:
                entities["rain"] = True
            elif "без дождя" in text_lower:
                entities["rain"] = False

            # Извлечение времени уведомления (например, "в 8 утра", "в 15:30")
            time_match = re.search(r'(?:в|к)\s*(\d{1,2}(?:[:.]\d{2})?)(?:\s*(?:утра|вечера)?)', text_lower)
            if time_match:
                try:
                    time_str = time_match.group(1).replace('.', ':') # Заменяем точку на двоеточие
                    # Добавляем нули, если время указано как '8' -> '08:00'
                    if ':' not in time_str:
                        time_str = f"{int(time_str):02d}:00"
                    elif len(time_str.split(':')[0]) == 1:
                        time_str = f"0{time_str}"
                    
                    # Проверяем, что это корректное время
                    datetime.strptime(time_str, "%H:%M") 
                    entities["notify_time"] = time_str
                except ValueError:
                    pass 

        return entities

    def process_text(self, text: str) -> Optional[Dict[str, Any]]: 
        doc = self.nlp(text)
        matches = self.matcher(doc)

        if matches:
            match_id, start, end = matches[0]
            # matched_command теперь уже будет в формате "/weather", "/forecast" и т.д.
            matched_command = self.nlp.vocab.strings[match_id] 

            entities = self._extract_entities(doc, matched_command)
            
            return {
                "command": matched_command, # Возвращаем "command" вместо "intent"
                "entities": entities
            }
        else:
            return None


if __name__ == "__main__":
    nlp_service = NlpService() 

    test_phrases = [
        "Какая погода в Москве?", 
        "Прогноз на завтра в Питере", 
        "Подписаться на дождь в Новосибирске в 8 утра", 
        "Отписаться от погоды в Казани", 
        "Хочу уведомления, когда температура выше 25 в Сочи", 
        "Ветер сильнее 10.5 м/с в Лондоне", 
        "Мои подписки", 
        "Привет",
        "Спасибо большое",
        "Что такое", # Неизвестный запрос
        "Изменить подписку в Минске на без дождя"
    ]

    for phrase in test_phrases:
        print(f"\nОбработка фразы: '{phrase}'")
        result = nlp_service.process_text(phrase)
        if result:
            print(f"  NLP-результат: {json.dumps(result, ensure_ascii=False, indent=2)}")
            
            # Имитация формирования команды для Kafka
            matched_command = result.get("command")
            nlp_entities = result.get("entities", {})

            simulated_command_for_kafka = ""
            if matched_command:
                simulated_command_for_kafka = matched_command # Берем команду напрямую
                
                if nlp_entities:
                    if "city" in nlp_entities and nlp_entities["city"]:
                        simulated_command_for_kafka += f" {nlp_entities['city']}"
                    
                    if "date" in nlp_entities and nlp_entities["date"] and matched_command == "/forecast":
                        simulated_command_for_kafka += f" {nlp_entities['date']}"
                    
                    if matched_command in ["/subscribe", "/modify_subscription"]:
                        # Добавляем все параметры подписки
                        params = []
                        if "condition" in nlp_entities and nlp_entities["condition"]:
                            params.append(nlp_entities["condition"]) # Например, "rain"
                        if "temp_above" in nlp_entities and nlp_entities["temp_above"] is not None:
                            params.append(f"temp_gt_{nlp_entities['temp_above']}")
                        if "rain" in nlp_entities and nlp_entities["rain"] is not None:
                            params.append(f"rain_{'true' if nlp_entities['rain'] else 'false'}")
                        if "wind_speed_gt" in nlp_entities and nlp_entities["wind_speed_gt"] is not None:
                            params.append(f"wind_gt_{nlp_entities['wind_speed_gt']}")
                        if "notify_time" in nlp_entities and nlp_entities["notify_time"]:
                            params.append(f"time_{nlp_entities['notify_time']}")
                        
                        if params:
                            simulated_command_for_kafka += " " + " ".join(params)

            else:
                simulated_command_for_kafka = "/unknown_nlp_query"

            print(f"  Имитированная команда для Kafka: '{simulated_command_for_kafka}'")
        else:
            print("  Команда не распознана.")