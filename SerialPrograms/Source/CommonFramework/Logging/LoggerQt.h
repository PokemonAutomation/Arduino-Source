/*  Logger Qt
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Logging_LoggerQt_H
#define PokemonAutomation_Logging_LoggerQt_H

#include <string>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/AbstractLogger.h"

class QString;

namespace PokemonAutomation{


class LoggerQt : public Logger{
public:
    virtual void log(const char* msg, Color color = Color()) = 0;
    virtual void log(const std::string& msg, Color color = Color()) = 0;
    virtual void log(const QString& msg, Color color = Color()) = 0;
};


//  Print as is. Use this to build other loggers.
LoggerQt& global_logger_raw();

//  Print with timestamp and a default tag. use this directly.
LoggerQt& global_logger_tagged();



class TaggedLogger : public LoggerQt{
public:
    TaggedLogger(LoggerQt& logger, std::string tag);

    LoggerQt& base_logger(){ return m_logger; }

    virtual void log(const char* msg, Color color = Color()) override;
    virtual void log(const std::string& msg, Color color = Color()) override;
    virtual void log(const QString& msg, Color color = Color()) override;

private:
    LoggerQt& m_logger;
    std::string m_tag;
};







}
#endif

