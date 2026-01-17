/*  Tagged Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A logger wrapper that prepends a timestamp and tag to each log message.
 *  Useful for categorizing log output from different components.
 */

#ifndef PokemonAutomation_Logging_TaggedLogger_H
#define PokemonAutomation_Logging_TaggedLogger_H

#include <string>
#include "AbstractLogger.h"

namespace PokemonAutomation{


// Wrapper around another logger to prepend a timestamp and tag string to each message.
// Format: "<timestamp> - [<tag>]: <message>"
//
// Example usage:
//   FileLogger file_logger(config);
//   TaggedLogger tagged(file_logger, "Network");
//   tagged.log("Connection established");
//   // Output: "2024-01-15 10:30:45.123 - [Network]: Connection established"
class TaggedLogger : public Logger{
public:
    // Construct a TaggedLogger that wraps the given base logger.
    // All log messages will be prefixed with a timestamp and the given tag.
    TaggedLogger(Logger& logger, std::string tag);

    // Get the underlying base logger.
    Logger& base_logger(){ return m_logger; }

    // Logger interface: prepends timestamp and tag, then forwards to base logger.
    virtual void log(const std::string& msg, Color color = Color()) override;

private:
    Logger& m_logger;
    std::string m_tag;
};


}
#endif
