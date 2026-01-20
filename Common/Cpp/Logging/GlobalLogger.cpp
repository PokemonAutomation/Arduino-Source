/*  Global Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "GlobalLogger.h"

namespace PokemonAutomation{


static FileLoggerConfig _global_logger_config;

void initialize_global_logger(FileLoggerConfig config){
    _global_logger_config = std::move(config);
    // auto& logger = global_logger_raw();
    // logger.log("Initialized logger with path " + config.file_path);
}

Logger& global_logger_raw(){
    auto get_config = [&](){
        if (_global_logger_config.file_path.size() == 0){
            _global_logger_config.file_path = ".";
        }
        return _global_logger_config;
    };
    static FileLogger logger(get_config());
    return logger;
}


}
