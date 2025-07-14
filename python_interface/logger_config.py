# logger_config.py

import logging
import os
from logging.handlers import RotatingFileHandler

def setup_logging(log_file='app.log', level=logging.INFO, max_bytes=10*1024*1024, backup_count=5):
    """
    Настраивает логирование для приложения.

    Args:
        log_file (str): Имя файла для логов.
        level (int): Минимальный уровень логирования (e.g., logging.INFO, logging.DEBUG).
    """
    # Определяем путь к директории для логов
    log_dir = 'logs'
    if not os.path.exists(log_dir):
        os.makedirs(log_dir)
    
    log_path = os.path.join(log_dir, log_file)

    # Создаем основной логгер приложения
    root_logger = logging.getLogger()
    root_logger.setLevel(level)

    # Избегаем добавления нескольких обработчиков при перезагрузке Uvicorn
    if not root_logger.handlers:
        # Консольный обработчик
        console_handler = logging.StreamHandler()
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        console_handler.setFormatter(formatter)
        root_logger.addHandler(console_handler)

        file_handler = RotatingFileHandler(
            log_path,
            maxBytes=max_bytes,
            backupCount=backup_count,
            encoding='utf-8'
        )
        file_handler.setFormatter(formatter)
        root_logger.addHandler(file_handler)

    # Настройка уровня для некоторых библиотек, которые могут быть слишком "многословными"
    logging.getLogger('uvicorn').setLevel(logging.WARNING)
    logging.getLogger('uvicorn.access').setLevel(logging.WARNING)
    logging.getLogger('httpx').setLevel(logging.WARNING)
    logging.getLogger('httpcore').setLevel(logging.WARNING)
    logging.getLogger('pymorphy3').setLevel(logging.INFO) 
    

    logging.info(f"Logging configured. Logs will be written to {log_path} and console.")
