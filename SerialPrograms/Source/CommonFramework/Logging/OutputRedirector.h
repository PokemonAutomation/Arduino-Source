/*  Output Redirector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Convenience wrapper around Common/Cpp/Logging/OutputRedirector that
 *  automatically uses the global logger.
 */

#ifndef PokemonAutomation_CommonFramework_Logging_OutputRedirector_H
#define PokemonAutomation_CommonFramework_Logging_OutputRedirector_H

#include "Common/Cpp/Logging/OutputRedirector.h"
#include "Logger.h"

namespace PokemonAutomation{


// Convenience class that redirects a stream to the global logger.
// This is a thin wrapper around Common/Cpp/Logging/OutputRedirector that
// automatically uses global_logger_raw() as the target logger.
//
// Example usage:
//   OutputRedirector redirect_stdout(std::cout, "stdout", Color());
//   OutputRedirector redirect_stderr(std::cerr, "stderr", COLOR_RED);
class GlobalOutputRedirector{
public:
    GlobalOutputRedirector(std::ostream& stream, std::string tag, Color color)
        : m_redirector(stream, global_logger_raw(), std::move(tag), color)
    {}

private:
    OutputRedirector m_redirector;
};


}
#endif
