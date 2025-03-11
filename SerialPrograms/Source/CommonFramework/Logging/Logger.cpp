/*  Logger
 *
 *  From: https://github.com/PokemonAutomation/
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

void TaggedLogger::log(const std::string& msg, Color color){
    std::string str =
        current_time_to_str() +
        " - [" + m_tag + "]: " +
        msg;
    m_logger.log(std::move(str), color);
}



class CommandLineLogger : public Logger{
public:
    CommandLineLogger(Logger& logger)
    : m_logger(logger) {}

    virtual void log(const std::string& msg, Color color = Color()) override{
        std::cout << msg << std::endl;
        m_logger.log(msg, color);
    }
    virtual void log(std::string&& msg, Color color = Color()) override{
        std::cout << msg << std::endl;
        m_logger.log(std::move(msg), color);
    }
    virtual void log(const char* msg, Color color = Color()) override{
        std::cout << msg << std::endl;
        m_logger.log(msg, color);
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

