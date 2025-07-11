# nlp_spacy/nlp_service.py

import spacy
from spacy.matcher import Matcher
import pymorphy3 # morphological analyzer for Russian and Ukrainian languages.
import logging 
from typing import Dict, Any
import datetime

from nlp_spacy.nlp_patterns import NLP_PATTERNS

from nlp_spacy.config import (
    SPACY_MODEL_NAME,
    CITY_PATTERNS,
    WEATHER_CONDITION_PATTERNS,
    DATE_PATTERNS,
    CONDITION_NORMALIZATION,
    DAY_MAPPINGS_NUM
)

# get the logger for this module
logger = logging.getLogger(__name__) 

class NlpService:
    def __init__(self):
        self.nlp = spacy.load(SPACY_MODEL_NAME)
        self.morph = pymorphy3.MorphAnalyzer()
        self.matcher = Matcher(self.nlp.vocab) 
        
        self._add_entity_ruler_patterns()
        self._add_intent_patterns_to_matcher()
        
        logger.info(f"spacy model '{SPACY_MODEL_NAME}' successfully loaded and patterns added.")

    def _add_entity_ruler_patterns(self):
        """adds rules for recognizing entities (cities, conditions, dates) using entityruler."""
        if "entity_ruler" not in self.nlp.pipe_names:
            ruler = self.nlp.add_pipe("entity_ruler", before="ner")
        else:
            ruler = self.nlp.get_pipe("entity_ruler")

        ruler.add_patterns(CITY_PATTERNS)
        ruler.add_patterns(WEATHER_CONDITION_PATTERNS)
        ruler.add_patterns(DATE_PATTERNS)
        logger.info(f"added {len(CITY_PATTERNS) + len(WEATHER_CONDITION_PATTERNS) + len(DATE_PATTERNS)} entityruler patterns.")

    def _add_intent_patterns_to_matcher(self):
        """adds patterns for recognizing intents to spacy matcher."""
        for intent, patterns in NLP_PATTERNS.items():
            self.matcher.add(intent, patterns)
        logger.info(f"added {len(NLP_PATTERNS)} intent categories to spacy matcher.")

    def process_text(self, text: str) -> Dict[str, Any]:
        doc = self.nlp(text.lower())

        intent = self._recognize_intent(doc)
        entities = self._extract_entities(doc)

        return {"command": intent, "entities": entities}

    def _recognize_intent(self, doc) -> str:
        """determines the message intent based on the matcher."""
        matches = self.matcher(doc)
        
        # sort matches by span length (longest first)
        matches_sorted = sorted(matches, key=lambda x: (x[2] - x[1]), reverse=True)

        for match_id, start, end in matches_sorted:
            span = doc[start:end]
            intent_name = self.nlp.vocab.strings[match_id]
            logger.debug(f"matched intent: {intent_name} with span: '{span.text}'")
            return intent_name
        
        return "unknown_text"

    def _extract_entities(self, doc) -> Dict[str, Any]:
        """extracts entities (city, date, conditions, etc.) from the text."""
        entities = {}

        # extract city entity, prioritizing entity ruler matches
        for ent in doc.ents:
            if ent.label_ == "CITY":
                parsed_city = self.morph.parse(ent.text)[0]
                entities["city"] = parsed_city.normal_form.capitalize()
                logger.debug(f"extracted city: {entities['city']}")
                break # assume only one city is relevant for now

        # extract date entity, prioritizing entity ruler matches
        for ent in doc.ents:
            if ent.label_ == "DATE":
                date_text = ent.text
                if date_text == "сегодня": # today
                    entities["date"] = datetime.date.today().isoformat()
                elif date_text == "завтра": # tomorrow
                    entities["date"] = (datetime.date.today() + datetime.timedelta(days=1)).isoformat()
                elif date_text == "послезавтра": # day after tomorrow
                    entities["date"] = (datetime.date.today() + datetime.timedelta(days=2)).isoformat()
                elif date_text in DAY_MAPPINGS_NUM: # specific day of the week
                    today_weekday = datetime.date.today().weekday()
                    target_weekday = DAY_MAPPINGS_NUM[date_text]
                    days_diff = (target_weekday - today_weekday + 7) % 7
                    # if the day is "monday" and today is monday, it means next monday
                    if days_diff == 0 and date_text not in ["сегодня", "завтра", "послезавтра"]:
                        days_diff = 7
                    entities["date"] = (datetime.date.today() + datetime.timedelta(days=days_diff)).isoformat()
                else:
                    entities["date"] = date_text # fallback for other date formats
                logger.debug(f"extracted date: {entities['date']}")
                break # assume only one date is relevant for now

        # extract weather condition entity, prioritizing entity ruler matches
        for ent in doc.ents:
            if ent.label_ == "WEATHER_CONDITION":
                normalized_condition = CONDITION_NORMALIZATION.get(ent.text, ent.text)
                entities["condition"] = normalized_condition
                logger.debug(f"extracted condition: {entities['condition']}")
                break # assume only one condition is relevant for now