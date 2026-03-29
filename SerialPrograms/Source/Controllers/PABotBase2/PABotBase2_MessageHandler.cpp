/*  PABotBase2 Message Handler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "PABotBase2_MessageHandler.h"

namespace PokemonAutomation{
namespace PABotBase2{


bool MessageHandler::should_print() const{
    return GlobalSettings::instance().LOG_EVERYTHING;
}
void MessageHandler::assert_size_eq(Logger& logger, uint16_t bytes, const MessageHeader* header) const{
    if (header->message_bytes != bytes){
        throw SerialProtocolException(
            logger, PA_CURRENT_FUNCTION,
            "Received Incorrect Response Size: Expected = " + std::to_string(bytes) + ", Actual = " + std::to_string(header->message_bytes)
        );
    }
}
void MessageHandler::assert_size_min(Logger& logger, uint16_t min_bytes, const MessageHeader* header) const{
    if (header->message_bytes < min_bytes){
        throw SerialProtocolException(
            logger, PA_CURRENT_FUNCTION,
            "Received too short Response: Minimum = " + std::to_string(min_bytes) + ", Actual = " + std::to_string(header->message_bytes)
        );
    }
}



}
}
