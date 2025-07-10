# nlp_app.py

import uvicorn
from fastapi import FastAPI
from pydantic import BaseModel
import sys
import os
import json 
from typing import Optional, Dict, Any

# Добавим Optional[Dict[str, Any]] для типа возвращаемого значения, чтобы быть точнее
# Это не влияет на работоспособность, но улучшает читаемость и подсказки типов

sys.path.append(os.path.dirname(os.path.abspath(__file__))) 
from nlp_service import NlpService 

app = FastAPI()
nlp_processor = NlpService()

class NLPRequest(BaseModel):
    user_id: int
    username: Optional[str] = None
    first_name: Optional[str] = None
    chat_id: int
    text: str

@app.post("/process_text")
async def process_text_endpoint(request: NLPRequest) -> Dict[str, Any]: # Добавили тип возвращаемого значения
    print(f"Received NLP request: {json.dumps(request.model_dump(), ensure_ascii=False)}")
    result = nlp_processor.process_text(request.text)
    
    if result:
        print(f"NLP processed result: {json.dumps(result, ensure_ascii=False)}")
        return {"command": result.get("command", ""), "entities": result.get("entities", {})}
    else:
        print("NLP did not recognize a command.")
        return {"command": "", "entities": {}} 

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)