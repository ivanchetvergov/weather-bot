# nlp_spacy/entity_extractor.py

import re
from datetime import datetime, timedelta
from typing import Dict, Any, Set
import spacy
from spacy.tokens import Doc
import pymorphy3 # # morphological analyzer for Russian and Ukrainian languages.


from nlp_spacy.config import (
    CITY_NORMALIZATION,         # for city normalization, if EntityRuler couldn't handle it
    CONDITION_NORMALIZATION,    # for weather condition normalization
    DAY_MAPPINGS_NUM            # for processing weekdays
)

class EntityExtractor:
    def __init__(self, nlp_model: spacy.Language, morph_analyzer: pymorphy3.MorphAnalyzer): # <-- added morph_analyzer to __init__
        self.nlp = nlp_model
        self.morph = morph_analyzer 
        
    def extract_entities(self, doc: Doc, matched_command: str) -> Dict[str, Any]:
        entities = {}
        text_lower = doc.text.lower()
        
        # --- 1. city extraction (priority: EntityRuler, then pymorphy3 normalization) ---
        found_city_ent = None
        for ent in doc.ents:
            if ent.label_ == "CITY": # looking for entities labeled as CITY by EntityRuler
                found_city_ent = ent
                break # take the first city found

        if found_city_ent:
            # using pymorphy3 to normalize the city form (lemmatization)
            parsed_city = self.morph.parse(found_city_ent.text)[0]
            # additional normalization from CITY_NORMALIZATION for abbreviations
            normalized_city = CITY_NORMALIZATION.get(parsed_city.normal_form.lower(), parsed_city.normal_form).capitalize()
            entities["city"] = normalized_city
            
        
        # --- 2. date extraction for the /forecast command ---
        found_date_ent = None
        for ent in doc.ents:
            if ent.label_ == "DATE": # looking for entities labeled as DATE by EntityRuler
                found_date_ent = ent
                break 

        if found_date_ent:
            date_text = found_date_ent.text.lower()
            current_date_val = None
            today = datetime.now().date() # get current date here

            if date_text == "сегодня": # today
                current_date_val = today.isoformat() # using isoformat for standardization
            elif date_text == "завтра": # tomorrow
                current_date_val = (today + timedelta(days=1)).isoformat()
            elif date_text == "послезавтра": # day after tomorrow
                current_date_val = (today + timedelta(days=2)).isoformat()
            elif date_text in DAY_MAPPINGS_NUM: # using DAY_MAPPINGS_NUM
                today_weekday = today.weekday() # monday is 0, sunday is 6
                target_weekday = DAY_MAPPINGS_NUM[date_text]
                days_diff = (target_weekday - today_weekday + 7) % 7
                # if the weekday matches the current one, but it's not "сегодня" (i.e., next week)
                if days_diff == 0 and "сегодня" not in text_lower:
                    days_diff = 7 
                current_date_val = (today + timedelta(days=days_diff)).isoformat()
            else:
                # attempting to parse DD.MM or DD.MM.YYYY, if EntityRuler didn't recognize this date format
                match_dm = re.match(r'(\d{1,2})\.(\d{1,2})$', date_text)
                match_dmy = re.match(r'(\d{1,2})\.(\d{1,2})\.(\d{4})$', date_text)
                
                if match_dmy:
                    day, month, year = map(int, match_dmy.groups())
                    try:
                        current_date_val = datetime(year, month, day).date().isoformat()
                    except ValueError:
                        pass # invalid date
                elif match_dm:
                    day, month = map(int, match_dm.groups())
                    try:
                        # if year not specified, use current year. account for year transition.
                        current_year = today.year
                        if month < today.month or (month == today.month and day < today.day):
                            current_year += 1 # if date is in the past but no year, assume next year
                        current_date_val = datetime(current_year, month, day).date().isoformat()
                    except ValueError:
                        pass # invalid date

            if current_date_val:
                entities["date"] = current_date_val

        # --- 3. weather condition extraction (priority: EntityRuler, then normalization) ---
        found_condition_ent = None
        for ent in doc.ents:
            if ent.label_ == "WEATHER_CONDITION": # looking for entities labeled as WEATHER_CONDITION by EntityRuler
                found_condition_ent = ent
                break # take the first condition found

        if found_condition_ent:
            # normalizing the extracted condition using CONDITION_NORMALIZATION
            normalized_condition = CONDITION_NORMALIZATION.get(found_condition_ent.text.lower(), found_condition_ent.text)
            entities["condition"] = normalized_condition

        # --- 4. extraction of specific parameters for subscription/tracking commands ---
        if matched_command in ["/subscribe", "/modify_subscription", "/track"]:
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
                    # if hours is a single digit, add 0 (e.g., "1:00" -> "01:00")
                    elif len(time_str.split(':')[0]) == 1:
                        time_str = f"0{time_str}"
                    
                    # validate that it's a correct time format in 24-hour format (%H:%M)
                    datetime.strptime(time_str, "%H:%M") # <-- fixed: from %h:%m to %H:%M
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