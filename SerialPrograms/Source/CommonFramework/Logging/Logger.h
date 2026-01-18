/*  Logger Qt
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Logging_Logger_H
#define PokemonAutomation_Logging_Logger_H

#include <string>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Logger/AbstractLogger.h"

namespace PokemonAutomation{

class TaggedLogger;

//  The base logger for the application. Use this to build other loggers.
//  Its implementation is defined in FileWindowLogger.cpp, writing each input
//  log into a log file named "<USER_FILE_PATH()>/<ApplicationName>.log".
//  It prints each input log string as is with no tag or timestamp.
Logger& global_logger_raw();

//  This logger wraps around `global_logger_raw()` to print each log with a 
//  timestamp and a default tag "Global". use this logger directly in the
//  application codebase.
TaggedLogger& global_logger_tagged();

//  This logger wraps around `global_logger_raw()` to also print the log
//  into command line using std::cout.
//  Useful for running command line tests.
Logger& global_logger_command_line();



// Wrapper around a another logger to log with a timestamp and given tag string.
// For the global TaggedLogger, use `global_logger_tagged()`.
class TaggedLogger : public Logger{
public:
    TaggedLogger(Logger& logger, std::string tag);

    Logger& base_logger(){ return m_logger; }

    virtual void log(const std::string& msg, Color color = Color()) override;

private:
    Logger& m_logger;
    std::string m_tag;
};







}
#endif

