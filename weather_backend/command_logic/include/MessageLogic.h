
void KafkaMessageService::handleTelegramMessage(const nlohmann::json& payload, const string& rawPayload) {
    cout << "    --> This is a general Telegram message." << endl;
    if (!dbClient_) set_DB("default");

    if (payload.contains("data") && payload["data"].is_object() &&
        payload["data"].contains("message") && payload["data"]["message"].is_object() &&
        payload["data"]["message"].contains("text") && payload["data"]["message"]["text"].is_string()) {
        string message_text = payload["data"]["message"]["text"].get<string>();
        cout << "      Message Text: " << message_text << endl;
    } else {
        cout << "    Message text not found in general Telegram message payload." << endl;
    }
}

void KafkaMessageService::handleStartCommand(long long telegram_user_id, const string& username, const string& first_name) {
    cout << "    --> Handling /start command for Telegram User ID: " << telegram_user_id << endl;

    if (telegram_user_id == 0) { // Telegram user IDs are positive non-zero
        cerr << "    ERROR: Cannot register user with invalid Telegram User ID (0)." << endl;
        return;
    }

    if (!dbClient_) set_DB("default");

    dbClient_->execSqlAsync(
        "INSERT INTO users (id, username, first_name) VALUES ($1, $2, $3) "
        "ON CONFLICT (id) DO UPDATE SET username = EXCLUDED.username, first_name = EXCLUDED.first_name, created_at = CURRENT_TIMESTAMP",
        
        // success callback
        [telegram_user_id, username, first_name](const drogon::orm::Result& result) {
            cout << "    User " << telegram_user_id
                      << " (" << (!username.empty() ? username : "N/A")
                      << ", " << first_name << ") registered/updated successfully in DB." << endl;
        },

        // error callback
        [telegram_user_id](const drogon::orm::DrogonDbException& e) {
            cerr << "    ERROR: Failed to register/update user " << telegram_user_id
                      << " in DB: " << e.base().what() << endl;
        },

        // sql parameters
        telegram_user_id,
        username.empty() ? nullptr : username.c_str(),
        first_name
    );
}
