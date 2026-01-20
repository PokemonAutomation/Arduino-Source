/*  Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Provides global logger accessors for the application.
 *  The core logging types (TaggedLogger, etc.) are in Common/Cpp/Logging/.
 */

#ifndef PokemonAutomation_Logging_Logger_H
#define PokemonAutomation_Logging_Logger_H

#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Logging/GlobalLogger.h"

namespace PokemonAutomation{


// This logger wraps around `global_logger_raw()` to print each log with a
// timestamp and a default tag "Global". Use this logger directly in the
// application codebase.
Logger& global_logger_tagged();

// This logger wraps around `global_logger_raw()` to also print the log
// into command line using std::cout.
// Useful for running command line tests.
Logger& global_logger_command_line();


}
#endif
