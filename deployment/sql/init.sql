CREATE TABLE users (
    id BIGINT PRIMARY KEY,
    username TEXT UNIQUE,
    first_name TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    default_city TEXT
);

CREATE TABLE messages (
    id SERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    command_text TEXT,
    text TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE subscriptions (
    id SERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    city TEXT NOT NULL,
    -- Условия
    temp_above FLOAT,
    temp_below FLOAT,
    rain_expected BOOLEAN, -- True/False/NULL (NULL = не задано, True = ожидается, False = не ожидается)
    snow_expected BOOLEAN,
    wind_speed_gt FLOAT,
    wind_speed_lt FLOAT,
    humidity_gt FLOAT,
    humidity_lt FLOAT,
    pressure_gt FLOAT,
    pressure_lt FLOAT,
    -- Параметры уведомления
    notify_time TIME, -- Конкретное время (для ежедневных уведомлений, например, 07:00:00)
    notify_interval_hours INTEGER, -- Интервал в часах (например, 3, 6, 12). Если задано notify_time, это игнорируется.
    notify_on_change BOOLEAN DEFAULT FALSE, -- Уведомлять только при изменении состояния условия/погоды
    last_notified_at TIMESTAMP, -- Время последнего отправленного уведомления по данной подписке
    last_notified_data JSONB, -- Данные, по которым было отправлено последнее уведомление (для 'notify_on_change')
    is_active BOOLEAN DEFAULT TRUE, -- Активна ли подписка. Можно деактивировать, а не удалять.
    -- Настройки контента
    detail_level TEXT DEFAULT 'short', -- 'short', 'full'
    units TEXT DEFAULT 'metric', -- 'metric', 'imperial'
    UNIQUE(user_id, city, notify_time, temp_above, temp_below, rain_expected, snow_expected, wind_speed_gt, wind_speed_lt, humidity_gt, humidity_lt, pressure_gt, pressure_lt) -- Обновлённая уникальность
);

CREATE TABLE trackings ( -- Переименована из alerts
    id SERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    city TEXT NOT NULL,
    -- Условия отслеживания (аналогично subscriptions, но для разового уведомления)
    temp_above FLOAT,
    temp_below FLOAT,
    rain_expected BOOLEAN,
    snow_expected BOOLEAN,
    wind_speed_gt FLOAT,
    wind_speed_lt FLOAT,
    humidity_gt FLOAT,
    humidity_lt FLOAT,
    pressure_gt FLOAT,
    pressure_lt FLOAT,
    -- Флаги состояния отслеживания
    is_active BOOLEAN DEFAULT TRUE, -- Активно ли отслеживание
    triggered_at TIMESTAMP, -- Время, когда условие сработало и уведомление было отправлено (NULL, если ещё не сработало)
    -- Дополнительная информация
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, -- Когда отслеживание было создано
    -- Уникальность для предотвращения дубликатов активных трекеров
    UNIQUE(user_id, city, temp_above, temp_below, rain_expected, snow_expected, wind_speed_gt, wind_speed_lt, humidity_gt, humidity_lt, pressure_gt, pressure_lt)
);

CREATE TABLE weather_cache (
    city TEXT PRIMARY KEY,
    timestamp TIMESTAMP NOT NULL,
    json_data JSONB NOT NULL
);