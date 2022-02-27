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


class SerialLogger : public Logger, public MessageLogger{
public:
    SerialLogger(Logger& logger);

    virtual void log(const char* msg, Color color = Color()) override;
    virtual void log(const std::string& msg, Color color = Color()) override;
    virtual void log(const QString& msg, Color color = Color()) override;
    virtual void log(std::string msg) override;

private:
    Logger& m_logger;
};



}
#endif
