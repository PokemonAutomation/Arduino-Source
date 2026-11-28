/*  Log Level
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  How important a log line is, and helpers to name and color each level.
 *  `Logger` (AbstractLogger.h) takes a level with each line.
 */

#ifndef PokemonAutomation_Logging_LogLevel_H
#define PokemonAutomation_Logging_LogLevel_H

#include <string>
#include <optional>
#include "Common/Cpp/Color.h"

namespace PokemonAutomation{


// How important a log line is, from least to most important.
// A logger can drop the lines below a minimum level: e.g. the command-line unit test
// logger (`global_logger_command_line()`) prints only Info lines and above by default,
// so the test log is not flooded with the result of every OCR read.
// `Logger::log()` without a level logs at Info.
//
// The names are PascalCase: an all-caps `ERROR` would clash with the `ERROR` macro of
// the Windows headers.
enum class LogLevel{
    // Details of each step, logged on every call, e.g. the result of every OCR read
    // or image match. Useful to debug, but they flood the log.
    Verbose,
    // Details to debug a problem that are not logged on every call, e.g. which model
    // file is loaded.
    Debug,
    // What the program is doing, e.g. which program starts, or a test result.
    Info,
    // Something unexpected that the program can handle, e.g. a retry or a skipped test.
    Warning,
    // Something failed, e.g. an operation cannot finish, or a test fails.
    Error,
    // A failure the program cannot recover from, e.g. right before it stops.
    Critical,
};


// Return the lowercase name of `level`, e.g. "warning" for `LogLevel::Warning`.
// The command-line option `--log-level` takes these names.
const char* log_level_to_str(LogLevel level);

// Return all level names from least to most important, e.g.
// "verbose, debug, info, warning, error, critical". For help and error messages.
std::string all_log_level_names();

// Parse a name from `log_level_to_str()`, e.g. "warning" -> `LogLevel::Warning`.
// Return std::nullopt if `name` is not a level name.
std::optional<LogLevel> parse_log_level(const std::string& name);

// Return the color of a line at `level` when the caller does not choose one:
// orange for Warning, red for Error and Critical, and no color (the default text
// color) for the rest. Used by `Logger::log(msg, level)`.
Color log_level_color(LogLevel level);


}
#endif
