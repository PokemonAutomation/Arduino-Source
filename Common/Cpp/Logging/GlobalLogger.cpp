/*  Global Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "GlobalLogger.h"

namespace PokemonAutomation{


// We should define this function at Main.cpp of each executable
// that uses the Logging/ library.
//
// This function is required by Common/Cpp/Logging/GlobalLogger.h:global_logger_raw() to initialize
// the global file logger.
// This function is called the first time `global_logger_raw()` is called to initialize the static
// local global file logger object.
//
// The advantage of such design is that:
// - This forces each executable's main.cpp to define how the logger is configured.
// - This ensures thread-safety on the global logger object. The static local `FileLogger` defined
//   in `global_logger_raw()` is guaranteed by C++ to be thread-safe when constructed.
FileLoggerConfig make_global_config();


Logger& global_logger_raw(){
    // Call a function `make_global_config()` that is not defined in 
    // this Logging/ library! To use this global logger, you must
    // define `make_global_config()` in the Main.cpp.
    static FileLogger logger(make_global_config());
    return logger;
}


}
