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


// Call this function at very beginning of the main function to initialize the global file logger.
// Call `global_logger_raw()` to retrieve the global file logger afterwards.
// If not called, a default logger with default file path of "." will be created.
void initialize_global_logger(FileLoggerConfig config);


// Return a global raw `FileLogger`. `FileLogger` is defined in FileLogger.h.
// "raw" here means the logger does not add any timestamp or tags to the incoming log lines.
// Suggest calling `initialize_global_logger()` at very beginning of the main function to
// initialize the logger first. Otherwise a default logger with default file path of "."
// will be created.
Logger& global_logger_raw();


}
#endif
