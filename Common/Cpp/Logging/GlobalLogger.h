/*  Global Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Provides a global file logger instance for application-wide logging.
 *  This is a Qt-free logger that can be used before Qt is initialized.
 */

#ifndef PokemonAutomation_Logging_GlobalLogger_H
#define PokemonAutomation_Logging_GlobalLogger_H

#include "FileLogger.h"

namespace PokemonAutomation{


// Return a global raw `FileLogger`. `FileLogger` is defined in FileLogger.h.
// "raw" here means the logger does not add any timestamp or tags to the incoming log lines.
//
// To initialize the global logger, implement `FileLoggerConfig make_global_config()` at
// Main.cpp to return the config for the global logger. If this function is not implemented,
// the program will not compile.
Logger& global_logger_raw();


}
#endif
