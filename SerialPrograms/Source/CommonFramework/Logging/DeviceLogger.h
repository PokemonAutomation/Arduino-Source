/*  Device Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Logging_DeviceLogger_H
#define PokemonAutomation_Logging_DeviceLogger_H

#include "ClientSource/Connection/MessageLogger.h"
#include "Logger.h"

namespace PokemonAutomation{


class SerialLogger : public TaggedLogger, public MessageLogger{
public:
    SerialLogger(Logger& logger, std::string tag);
    using TaggedLogger::log;
    virtual void log(std::string msg) override;
};



}
#endif
