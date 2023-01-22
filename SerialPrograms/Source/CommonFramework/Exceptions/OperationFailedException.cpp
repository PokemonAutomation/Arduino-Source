/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "OperationFailedException.h"

namespace PokemonAutomation{




OperationFailedException::OperationFailedException(Logger& logger, std::string message)
    : m_message(message)
{
    logger.log(std::string(OperationFailedException::name()) + ": " + m_message, COLOR_RED);
}





}
