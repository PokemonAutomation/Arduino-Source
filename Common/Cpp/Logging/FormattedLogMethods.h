/*  Formatted Log Methods
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Log methods named after the log levels, which build the line with
 *  `std::format()`, e.g. `logger.info("Read {} at {:.2f}", name, x);`.
 */

#ifndef PokemonAutomation_Logging_FormattedLogMethods_H
#define PokemonAutomation_Logging_FormattedLogMethods_H

#include <format>
#include <string>
#include <utility>
#include "Common/Cpp/Color.h"
#include "LogLevel.h"

namespace PokemonAutomation{


// Add the log methods `verbose()`, `debug()`, `info()`, `warning()`, `error()` and
// `critical()` to a class. Each one builds the line from a format string and its
// arguments with `std::format()`, and logs it at the level in its name:
//
//   logger.info("Read {} at {:.2f}, {:.2f}", name, x, y);
//   // Logs "Read gastly at 0.63, 0.44" at LogLevel::Info.
//
// The line gets the level's color from `log_level_color()`: orange for Warning, red
// for Error and Critical, and the default text color for the rest. To choose another
// color, pass it before the format string:
//
//   env.console.info(COLOR_BLUE, "Score: {:.3f}", score);
//
// The color and the level come before the arguments because the arguments at the end
// can be of any number and type, so nothing can follow them. That is why the level is
// in the method's name and the color is an optional first argument.
//
// The format string must be known at compile time, e.g. a string literal. The compiler
// then checks it against the arguments, so `info("{} and {}", a)` does not compile.
// To print a brace, write it twice: "{{" prints "{". To log a string built at runtime,
// use `log(msg, level)` or `log(msg, color, level)` instead.
// For the format syntax, see https://en.cppreference.com/w/cpp/utility/format/spec.
//
// `Logger`, `VideoStream` (e.g. `env.console`) and `ProgramEnvironment` (`env`) get
// these methods by inheriting this class. It uses CRTP: the class that inherits it
// passes itself as `Derived`, e.g. `class Logger : public FormattedLogMethods<Logger>`.
// `Derived` must have a public method `log(const std::string& msg, Color color,
// LogLevel level)`. Every method here calls it with the formatted line.
template <typename Derived>
class FormattedLogMethods{
public:
    template <typename... Args>
    void verbose(std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Verbose, std::format(format, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void verbose(Color color, std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Verbose, color, std::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void debug(std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Debug, std::format(format, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void debug(Color color, std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Debug, color, std::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void info(std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Info, std::format(format, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void info(Color color, std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Info, color, std::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void warning(std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Warning, std::format(format, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void warning(Color color, std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Warning, color, std::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void error(std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Error, std::format(format, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void error(Color color, std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Error, color, std::format(format, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void critical(std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Critical, std::format(format, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void critical(Color color, std::format_string<Args...> format, Args&&... args){
        log_line(LogLevel::Critical, color, std::format(format, std::forward<Args>(args)...));
    }

private:
    // Log `msg` at `level`, in the level's color.
    void log_line(LogLevel level, const std::string& msg){
        log_line(level, log_level_color(level), msg);
    }
    // Log `msg` at `level`, in `color`.
    void log_line(LogLevel level, Color color, const std::string& msg){
        static_cast<Derived&>(*this).log(msg, color, level);
    }
};


}
#endif
