/*  Inference Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_InferenceException_H
#define PokemonAutomation_CommonFramework_InferenceException_H

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Logging/LoggerQt.h"

namespace PokemonAutomation{


class InferenceException : public StringException{
public:
    InferenceException(const char* location, LoggerQt& logger, const char* message);
    InferenceException(const char* location, LoggerQt& logger, std::string message);
#ifdef QT_VERSION
    InferenceException(const char* location, LoggerQt& logger, const QString& message);
#endif

    virtual const char* type() const{
        return "InferenceException";
    }
};
#define PA_THROW_InferenceException(logger, message)    \
    throw InferenceException(__PRETTY_FUNCTION__, logger, message)




}
#endif
