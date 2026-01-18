/*  Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/Time.h"
#include "Common/Cpp/Logging/TaggedLogger.h"
#include "Logger.h"

#include <iostream>

namespace PokemonAutomation{


Logger& global_logger_tagged(){
    static TaggedLogger logger(global_logger_raw(), "Global");
    return logger;
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

