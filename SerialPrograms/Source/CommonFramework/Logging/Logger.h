/*  Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Logging_Logger_H
#define PokemonAutomation_Logging_Logger_H

#include <string>
#include "Common/Cpp/Color.h"

class QString;

namespace PokemonAutomation{


class Logger{
public:
    virtual void log(const char* msg, Color color = Color()) = 0;
    virtual void log(const std::string& msg, Color color = Color()) = 0;
    virtual void log(const QString& msg, Color color = Color()) = 0;
};


//  Print as is. Use this to build other loggers.
Logger& global_logger_raw();

//  Print with timestamp and a default tag. use this directly.
Logger& global_logger_tagged();



class TaggedLogger : public Logger{
public:
    TaggedLogger(Logger& logger, std::string tag);

    Logger& base_logger(){ return m_logger; }

    virtual void log(const char* msg, Color color = Color()) override;
    virtual void log(const std::string& msg, Color color = Color()) override;
    virtual void log(const QString& msg, Color color = Color()) override;

private:
    Logger& m_logger;
    std::string m_tag;
};







//std::string combine_string(const char* a, const char* b);
//std::string combine_string(const char* a, const std::string& b);
//QString combine_string(const char* a, const QString& b);

//#define PA_THROW_AND_LOG_StringException(message){  \
//    global_logger().log(combine_string(__PRETTY_FUNCTION__, message), COLOR_RED); \
//    throw StringException(__PRETTY_FUNCTION__, message);    \
//}



}
#endif

