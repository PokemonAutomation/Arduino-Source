/*  Inference Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "InferenceException.h"

namespace PokemonAutomation{


InferenceException::InferenceException(const char* location, Logger& logger, const char* message)
    : StringException(location, message)
{
    logger.log(message, COLOR_RED);
}



}
