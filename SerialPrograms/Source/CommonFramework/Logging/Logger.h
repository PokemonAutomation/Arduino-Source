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

namespace PokemonAutomation{


// This logger wraps around `global_logger_raw()` to print each log with a
// timestamp and a default tag "Global". Use this logger directly in the
// application codebase.
Logger& global_logger_tagged();

// This logger wraps around `global_logger_raw()` to also print the log
// into command line (stdout).
// Useful for running command line tests.
// It prints only lines at or above a minimum level, `LogLevel::Info` by default,
// so the test log is not flooded with details like every OCR read result.
// Lines below the level still go to `global_logger_raw()`, i.e. the log file.
Logger& global_logger_command_line();

// Set the minimum level of the lines `global_logger_command_line()` prints, e.g.
// `LogLevel::Verbose` to print every line. Set by the command-line option
// `--log-level` of the command-line test mode.
void set_command_line_log_level(LogLevel level);


class BooleanCheckBoxOption;
BooleanCheckBoxOption& LOG_EVERYTHING();


}
#endif
