/*  Multi-Console Errors
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "MultiConsoleErrors.h"

namespace PokemonAutomation{



void MultiConsoleErrorState::report_unrecoverable_error(LoggerQt& logger, const std::string& msg){
    logger.log(msg, COLOR_RED);
    m_unrecoverable_error.store(true, std::memory_order_release);
    throw OperationFailedException(logger, msg);
}
void MultiConsoleErrorState::check_unrecoverable_error(LoggerQt& logger){
    if (m_unrecoverable_error.load(std::memory_order_acquire)){
        logger.log("Unrecoverable error reported from a different console. Breaking out.", COLOR_RED);
        throw ProgramCancelledException();
    }
}



}
