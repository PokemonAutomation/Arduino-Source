/*  Logger Qt
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Logging_Logger_H
#define PokemonAutomation_Logging_Logger_H

#include <string>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/AbstractLogger.h"

namespace PokemonAutomation{



//  Print as is. Use this to build other loggers.
Logger& global_logger_raw();

//  Print with timestamp and a default tag. use this directly.
Logger& global_logger_tagged();

//  Print log also to command line. Useful for running command line tests.
Logger& global_logger_command_line();



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

