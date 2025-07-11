# python_bot/nlp_client.py

import httpx
import logging
import json
from typing import Dict, Any, Optional

from .config import NLP_SERVICE_URL

logger = logging.getLogger(__name__)

http_client = httpx.AsyncClient()

async def call_nlp_service(
    user_id: int, username: str, first_name: str, chat_id: int, text: str
) -> Dict[str, Any]:
    """
    calls the external nlp service and returns the recognized intent and entities.
    """
    nlp_payload_to_service = {
        "user_id": user_id,
        "username": username,
        "first_name": first_name,
        "chat_id": chat_id,
        "text": text
    }
    
    logger.debug(f"sending to nlp service: {json.dumps(nlp_payload_to_service, ensure_ascii=False)}")
    try:
        response = await http_client.post(NLP_SERVICE_URL, json=nlp_payload_to_service, timeout=5.0)
        response.raise_for_status() # will raise an exception for 4xx/5xx responses

        nlp_service_response = response.json()
        logger.debug(f"received from nlp service: {json.dumps(nlp_service_response, ensure_ascii=False)}")

        # the nlp service should return "command" (as intent) and "entities"
        return {
            "command": nlp_service_response.get("command", "unknown_nlp_intent"),
            "entities": nlp_service_response.get("entities", {})
        }
    except httpx.RequestError as exc:
        logger.error(f"http request error to nlp service for {exc.request.url!r}: {exc}")
        return {"command": "nlp_error", "entities": {"error_type": "request_error", "details": str(exc)}}
    except httpx.HTTPStatusError as exc:
        logger.error(f"error response {exc.response.status_code} from {exc.request.url!r}: {exc.response.text}")
        return {"command": "nlp_error", "entities": {"error_type": "status_error", "status_code": exc.response.status_code, "details": exc.response.text}}
    except json.JSONDecodeError:
        logger.error(f"error decoding json response from nlp service: {response.text}")
        return {"command": "nlp_error", "entities": {"error_type": "json_decode_error", "details": response.text}}
    except Exception as e:
        logger.error(f"an unexpected error occurred in call_nlp_service: {e}")
        return {"command": "nlp_error", "entities": {"error_type": "unexpected_error", "details": str(e)}}

async def close_nlp_client():
    """closes the http client for the nlp service."""
    await http_client.aclose()
    logger.info("http client for nlp service closed.")