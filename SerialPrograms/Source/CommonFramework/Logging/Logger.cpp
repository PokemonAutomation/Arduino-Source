/*  Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>
#include "ClientSource/Libraries/Logging.h"
#include "Logger.h"

#include <iostream>

namespace PokemonAutomation{


Logger& global_logger_tagged(){
    static TaggedLogger logger(global_logger_raw(), "Global");
    return logger;
}


TaggedLogger::TaggedLogger(Logger& logger, std::string tag)
    : m_logger(logger)
    , m_tag(std::move(tag))
{}

void TaggedLogger::log(const char* msg, Color color){
    log(std::string(msg), color);
}
void TaggedLogger::log(const std::string& msg, Color color){
    std::string str =
        current_time_to_str() +
        " - [" + m_tag + "]: " +
        msg;
    m_logger.log(str, color);
}



class CommandLineLogger : public Logger{
public:
    CommandLineLogger(Logger& logger)
    : m_logger(logger) {}

    virtual void log(const char* msg, Color color = Color()) override{
        m_logger.log(msg, color);
        std::cout << msg << std::endl;
    }
    virtual void log(const std::string& msg, Color color = Color()) override{
        log(msg.c_str(), color);
    }

private:
    Logger& m_logger;
    std::string m_tag;
};


Logger& global_logger_command_line(){
    static CommandLineLogger logger(global_logger_raw());
    return logger;
}


}

