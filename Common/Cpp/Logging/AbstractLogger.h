/*  Abstract Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AbstractLogger_H
#define PokemonAutomation_AbstractLogger_H

#include <string>
#include "Common/Cpp/Color.h"
#include "LogLevel.h"

namespace PokemonAutomation{


struct LogLine{
    Color color;
    std::string text;
};


// Abstract base class as a logger interface
class Logger{
public:
    // Log a line at `LogLevel::Info`.
    virtual void log(const std::string& msg, Color color = Color()) = 0;
    virtual void log(std::string&& msg, Color color = Color()){
        log((const std::string&)msg, color);
    }
    virtual void log(const char* msg, Color color = Color()){
        log(std::string(msg), color);
    }

    // Log a line at `level`, e.g.
    //   logger.log("OCR Text: \"0263\" -> 263", COLOR_BLUE, LogLevel::Verbose);
    // The default keeps lines of every level: it calls `log(msg, color)`.
    // Override it in a logger that drops lines by level, and in a logger that wraps
    // another logger, to pass the level on (e.g. `TaggedLogger`, `MultiOutputLogger`).
    virtual void log(const std::string& msg, Color color, LogLevel level){
        (void)level;
        log(msg, color);
    }

    // Log a line at `level`, in the level's color from `log_level_color()`, e.g.
    //   logger.log("Unable to read the stat box.", LogLevel::Error);   // red
    void log(const std::string& msg, LogLevel level){
        log(msg, log_level_color(level), level);
    }
};


}
#endif
