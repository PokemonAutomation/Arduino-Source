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
InferenceException::InferenceException(const char* location, Logger& logger, std::string message)
    : StringException("InferenceException", location, std::move(message))
{
    logger.log(message, COLOR_RED);
}
#ifdef QT_VERSION
InferenceException::InferenceException(const char* location, Logger& logger, const QString& message)
    : StringException("InferenceException", location, message.toUtf8().data())
{
    logger.log(message, COLOR_RED);
}
#endif



}
