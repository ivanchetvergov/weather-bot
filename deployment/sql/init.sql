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
    temp_above FLOAT,
    rain BOOLEAN,
    wind_speed_gt FLOAT,
    notify_time TIME,
    UNIQUE(user_id, city)
);

CREATE TABLE alerts (
    id SERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    city TEXT NOT NULL,
    condition TEXT NOT NULL,
    sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE weather_cache (
    city TEXT PRIMARY KEY,
    timestamp TIMESTAMP NOT NULL,
    json_data JSONB NOT NULL
);