/*  Multi-Console Errors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "MultiConsoleErrors.h"

namespace PokemonAutomation{



void MultiConsoleErrorState::report_unrecoverable_error(VideoStream& stream, std::string msg){
    stream.log(msg, COLOR_RED);
    bool expected = false;
    if (m_unrecoverable_error.compare_exchange_strong(expected, true)){
        m_message = msg;
    }
    OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, std::move(msg), stream);
}
void MultiConsoleErrorState::check_unrecoverable_error(Logger& logger){
    if (m_unrecoverable_error.load(std::memory_order_acquire)){
        logger.log("Unrecoverable error reported from a different console. Breaking out.", COLOR_RED);
        throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, m_message);
    }
}



}
