/*  Log Level
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "LogLevel.h"

namespace PokemonAutomation{


const char* log_level_to_str(LogLevel level){
    switch (level){
    case LogLevel::Verbose:
        return "verbose";
    case LogLevel::Debug:
        return "debug";
    case LogLevel::Info:
        return "info";
    case LogLevel::Warning:
        return "warning";
    case LogLevel::Error:
        return "error";
    case LogLevel::Critical:
        return "critical";
    }
    return "unknown";
}

std::string all_log_level_names(){
    std::string names;
    for (int i = (int)LogLevel::Verbose; i <= (int)LogLevel::Critical; i++){
        if (!names.empty()){
            names += ", ";
        }
        names += log_level_to_str((LogLevel)i);
    }
    return names;
}

std::optional<LogLevel> parse_log_level(const std::string& name){
    for (int i = (int)LogLevel::Verbose; i <= (int)LogLevel::Critical; i++){
        if (name == log_level_to_str((LogLevel)i)){
            return (LogLevel)i;
        }
    }
    return std::nullopt;
}

Color log_level_color(LogLevel level){
    switch (level){
    case LogLevel::Warning:
        return COLOR_ORANGE;
    case LogLevel::Error:
    case LogLevel::Critical:
        return COLOR_RED;
    default:
        return Color();
    }
}


}
