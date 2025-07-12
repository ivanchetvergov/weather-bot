# nlp_spacy/nlp_service.py

import spacy
from spacy.matcher import Matcher
import pymorphy3 # morphological analyzer for Russian and Ukrainian languages.
import logging 
from typing import Dict, Any

from nlp_spacy.nlp_patterns import NLP_PATTERNS
from nlp_spacy.entity_extractor import EntityExtractor

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

        self.entity_extractor = EntityExtractor(self.nlp, self.morph)
        
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
        entities = self.entity_extractor.extract_entities(doc, intent)

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
