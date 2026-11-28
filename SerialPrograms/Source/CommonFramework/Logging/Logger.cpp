/*  Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstdio>
#include <atomic>
#include "Common/Cpp/Logging/TaggedLogger.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Logger.h"

namespace PokemonAutomation{


Logger& global_logger_tagged(){
    static TaggedLogger logger(global_logger_raw(), "Global");
    return logger;
}


// A logger that writes to both the base logger and stdout.
// Useful for command-line tests.
// To keep the console output short, it prints to stdout only lines at or above a
// minimum level (Info by default, see `set_command_line_log_level()`). Every line,
// e.g. each Verbose OCR read result, still goes to the base logger, i.e. the
// program's log file.
class CommandLineLogger : public Logger{
public:
    CommandLineLogger(Logger& logger)
        : m_logger(logger)
    {}

    void set_min_level(LogLevel level){
        m_min_level.store(level, std::memory_order_relaxed);
    }

    virtual void log(const std::string& msg, Color color = Color()) override{
        print(msg.c_str(), LogLevel::Info);
        m_logger.log(msg, color);
    }
    virtual void log(std::string&& msg, Color color = Color()) override{
        print(msg.c_str(), LogLevel::Info);
        m_logger.log(std::move(msg), color);
    }
    virtual void log(const char* msg, Color color = Color()) override{
        print(msg, LogLevel::Info);
        m_logger.log(msg, color);
    }
    virtual void log(const std::string& msg, Color color, LogLevel level) override{
        print(msg.c_str(), level);
        m_logger.log(msg, color, level);
    }

private:
    //  Print `msg` to stdout if `level` is at or above the minimum level.
    void print(const char* msg, LogLevel level) const{
        if (level >= m_min_level.load(std::memory_order_relaxed)){
            printf("%s\n", msg);
        }
    }

private:
    Logger& m_logger;
    std::atomic<LogLevel> m_min_level = LogLevel::Info;
};


static CommandLineLogger& command_line_logger(){
    static CommandLineLogger logger(global_logger_raw());
    return logger;
}
Logger& global_logger_command_line(){
    return command_line_logger();
}
void set_command_line_log_level(LogLevel level){
    command_line_logger().set_min_level(level);
}





BooleanCheckBoxOption& LOG_EVERYTHING(){
    static BooleanCheckBoxOption option(
        "<b>Log Everything:</b><br>Log everything to the output window and output log. Will be very spammy.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    );
    return option;
}










}
