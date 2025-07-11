# nlp_spacy/entity_extractor.py

import re
from datetime import datetime, timedelta
from typing import Dict, Any, Set
import spacy
from spacy.tokens import Doc

from .config import KNOWN_CITIES, CONDITION_KEYWORDS, DAY_MAPPINGS

class EntityExtractor:
    def __init__(self, nlp_model: spacy.Language):
        self.nlp = nlp_model
        self.known_cities_lower = set(KNOWN_CITIES)
    
    def extract_entities(self, doc: Doc, matched_command: str) -> Dict[str, Any]:
        entities = {}
        text_lower = doc.text.lower()
        found_city = None

        # attempt to find city among spacy's named entities
        for ent in doc.ents:    
            if ent.label_ in ("city", "loc", "gpe"): # check common entity labels for locations
                ent_text_lower = ent.text.lower() 
                if ent_text_lower in self.known_cities_lower: # prioritize exact matches from known cities
                    found_city = ent_text_lower
                    break
                
                # check if the lemma of the entity is a known city
                if self.nlp.vocab.strings[ent.label_] == "city" and ent.lemma_.lower() in self.known_cities_lower:
                    found_city = ent.lemma_.lower()
                    break

        # if no city found via spacy ents, try searching in lemmatized text
        if not found_city:
            lemmatized_text_tokens = [token.lemma_.lower() for token in doc]
            lemmatized_text_string = " ".join(lemmatized_text_tokens)

            for city in self.known_cities_lower: 
                # use regex to find whole word matches for cities in the lemmatized text
                if re.search(r'\b' + re.escape(city) + r'\b', lemmatized_text_string):
                    found_city = city
                    break
        
        if found_city:
            entities["city"] = found_city

        # --- extract date for /forecast command ---
        if matched_command == "/forecast":
            today = datetime.now().date()
            
            if "сегодня" in text_lower: # today
                entities["date"] = today.strftime("%y-%m-%d")
            elif "завтра" in text_lower: # tomorrow
                entities["date"] = (today + timedelta(days=1)).strftime("%y-%m-%d")
            elif "послезавтра" in text_lower: # day after tomorrow
                entities["date"] = (today + timedelta(days=2)).strftime("%y-%m-%d")
            
            # search for days of the week
            for day_name, day_index in DAY_MAPPINGS.items():
                if day_name in text_lower:
                    current_day_of_week = today.weekday() # monday is 0 and sunday is 6
                    days_ahead = (day_index - current_day_of_week + 7) % 7
                    # if the day is the same as today, assume next week's day unless "today" is explicitly mentioned
                    if days_ahead == 0 and "сегодня" not in text_lower and "на сегодня" not in text_lower: 
                        days_ahead = 7 
                    
                    target_date = today + timedelta(days=days_ahead)
                    entities["date"] = target_date.strftime("%y-%m-%d")
                    break # assume only one date is specified

        # --- extract conditions and parameters for subscriptions/tracking ---
        if matched_command in ["/subscribe", "/modify_subscription", "/track"]:
            # extract weather condition keywords
            for keyword, condition_value in CONDITION_KEYWORDS.items():
                if keyword in text_lower:
                    entities["condition"] = condition_value
                    break
            
            # extract "temperature above x"
            temp_above_match = re.search(r'(?:температура|теплее)\s*(?:выше|больше)\s*(\d+)', text_lower)
            if temp_above_match:
                try:
                    entities["temp_above"] = float(temp_above_match.group(1))
                except ValueError:
                    pass

            # extract "wind stronger than y"
            wind_speed_match = re.search(r'(?:ветер)\s*(?:сильнее|быстрее)\s*(\d+\.?\d*)', text_lower)
            if wind_speed_match:
                try:
                    entities["wind_speed_gt"] = float(wind_speed_match.group(1))
                except ValueError:
                    pass
            
            # extract "rain" / "no rain"
            if "дождь" in text_lower and "без" not in text_lower: # "rain" and not "without"
                entities["rain"] = True
            elif "без дождя" in text_lower: # "without rain"
                entities["rain"] = False

            # extract notification time (e.g., "at 8 am", "at 15:30")
            time_match = re.search(r'(?:в|к)\s*(\d{1,2}(?:[:.]\d{2})?)(?:\s*(?:утра|вечера)?)', text_lower)
            if time_match:
                try:
                    time_str = time_match.group(1).replace('.', ':') # replace dot with colon for parsing
                    # add leading zeros if time is '8' -> '08:00'
                    if ':' not in time_str:
                        time_str = f"{int(time_str):02d}:00"
                    elif len(time_str.split(':')[0]) == 1: # if 1:00 instead of 01:00
                        time_str = f"0{time_str}"
                    
                    # validate that it's a correct time format
                    datetime.strptime(time_str, "%h:%m") 
                    entities["notify_time"] = time_str
                except ValueError:
                    pass 
        
        # --- extract id for /unsubscribe and /untrack commands ---
        if matched_command in ["/unsubscribe", "/untrack"]:
            # look for numbers in the text, assuming they are ids
            id_match = re.search(r'\b(\d+)\b', text_lower)
            if id_match:
                entities["id"] = int(id_match.group(1))
            elif "все" in text_lower or "all" in text_lower: # "all" for bulk cancellation
                entities["id"] = "all" 
                
        return entities