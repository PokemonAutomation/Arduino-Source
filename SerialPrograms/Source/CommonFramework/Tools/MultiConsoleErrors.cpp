/*  Multi-Console Errors
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "MultiConsoleErrors.h"

namespace PokemonAutomation{



void MultiConsoleErrorState::report_unrecoverable_error(Logger& logger, std::string msg){
    logger.log(msg, COLOR_RED);
    bool expected = false;
    if (m_unrecoverable_error.compare_exchange_strong(expected, true)){
        m_message = msg;
    }
    throw OperationFailedException(logger, std::move(msg));
}
void MultiConsoleErrorState::check_unrecoverable_error(Logger& logger){
    if (m_unrecoverable_error.load(std::memory_order_acquire)){
        logger.log("Unrecoverable error reported from a different console. Breaking out.", COLOR_RED);
        throw OperationFailedException(logger, m_message);
    }
}



}
