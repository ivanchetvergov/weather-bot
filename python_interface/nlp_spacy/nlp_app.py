# nlp_spacy/nlp_app.py

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import logging

from nlp_spacy.nlp_service import NlpService
from logger_config import setup_logging

setup_logging() 
logger = logging.getLogger(__name__)

app = FastAPI() # init app w FastAPI
nlp_processor = NlpService() # initialize the nlp service

class TextInput(BaseModel): 
    text: str # the text string to be processed

@app.post("/process_text")
async def process_text(input_data: TextInput):
    """
    api endpoint to process incoming text.
    it takes a text string, processes it using the nlp service, and returns the extracted intent and entities.
    """
    try:
        logger.info(f"received request for text: '{input_data.text}'")
        result = nlp_processor.process_text(input_data.text)
        logger.info(f"processed text: '{input_data.text}', result: {result}")
        return result
    except Exception as e:
        logger.exception(f"error processing text: '{input_data.text}'")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/health")
async def health_check():
    """
    api endpoint for health checks.
    returns a simple status message to indicate that the service is running.
    """
    return {"status": "ok", "message": "nlp service is running"}