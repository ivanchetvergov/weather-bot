#pragma once

#include <cppkafka/cppkafka.h>  
#include <nlohmann/json.hpp>    
#include <string>               
#include <iostream>             
#include <map>                  
#include <memory>               
#include <optional>             
#include <future>               
#include <set>                  

#include "KafkaResponseSender.h"  // for sending messages back to telegram
#include "TelegramUpdateParser.h" // for parsing incoming kafka messages into dtos
#include "ICommandLogic.h"        // interface for command execution logic
#include "DataBaseService.h"      // database service for interacting with postgresql

/**
 * @brief manages kafka message consumption and dispatching to command logic handlers.
 * it parses incoming kafka messages, interacts with the database, and dispatches commands.
 */
class KafkaMessageService {
public:
    KafkaMessageService();

    // * @brief sets the KafkaResponseSender for sending replies to telegram.
    void set_ResponseSender(KafkaResponseSenderPtr response_sender);

    // * @brief sets the database service for interacting with postgresql.
    void set_DbService(PgDbServicePtr db_service);

    // * @brief registers a command logic handler for a specific command name.
    void registerCommandLogic(const std::string& command_name, ICommandLogicPtr logic);

    // * @brief oarse the message and dispatches it to the appropriate handler based on its event type.
    void processMessage(const cppkafka::Message& msg);

private:

    // * @brief struct for holding asynchronous processing state of a kafka message.
    struct AsyncProcessingState {
            ParsedTelegramMessage parsed_msg;             // the parsed telegram message dto
            std::optional<std::string> resolved_city;     // the city determined for the command (from nlp or default)
            
            KafkaMessageService* service_ptr;              // pointer to the KafkaMessageService instance for callbacks

            // the command string to be saved to db and used for dispatch (e.g., "/weather Moscow")
            std::string command_text_for_db_and_dispatch; 

            AsyncProcessingState(const ParsedTelegramMessage& msg, KafkaMessageService* svc)
                : parsed_msg(msg), service_ptr(svc) {}
    };


    // * @brief dispatches a command to its registered ICommandLogic handler.
    void dispatchCommand(const std::string& command_name,
                         const nlohmann::json& payload,
                         long long telegram_user_id,
                         const std::string& message_text,
                         const std::string& username,
                         const std::string& first_name);

    
    // * @brief handles incoming telegram messages after parsing.
    void handleTelegramMessage(const ParsedTelegramMessage& parsed_msg);

    // * @brief handles responses from the weather api (e.g., for caching).
    void handleWeatherApiResponse(const ParsedTelegramMessage& parsed_msg);

    // * @brief first checks nlp_entities for a city, then falls back to the user's default city from the db.
    std::future<std::optional<std::string>> getCityForCommand(long long telegram_user_id,
                                                            const nlohmann::json& nlp_entities); 

    // * @brief sends an error message back to the specified telegram user.
    void sendErrorMessageToUser(long long telegram_user_id, const std::string& error_message);


    // * @brief initiates the asynchronous processing chain for a telegram message.
    void processTelegramMessageAsync(std::shared_ptr<AsyncProcessingState> state);

    // * @param state a shared pointer to the AsyncProcessingState.
    void step1_upsertUser(std::shared_ptr<AsyncProcessingState> state);

    // * @brief asynchronous step 2: resolves the city for the command and checks if it meets requirements.
    void step2_resolveCityAndCheckRequirements(std::shared_ptr<AsyncProcessingState> state);


    // * @brief asynchronous step 3: inserts the message record into the database and prepares the payload for dispatch.
    void step3_insertMessageAndPreparePayload(std::shared_ptr<AsyncProcessingState> state);

    // * @brief asynchronous step 4: dispatches the command to the appropriate logic handler.
    void step4_dispatchCommand(std::shared_ptr<AsyncProcessingState> state);

    TelegramUpdateParser messageParser_;                        // parser for incoming kafka messages
    PgDbServicePtr dbService_;                                  // database service instance
    KafkaResponseSenderPtr responseSender_;                     // service for sending responses back
    std::map<std::string, ICommandLogicPtr> commandLogics_;     // map of command names to their logic handlers
    std::set<std::string> commandsRequiringCity_;               // set of commands that explicitly need a city
};

using KafkaMessageServicePtr = std::shared_ptr<KafkaMessageService>;