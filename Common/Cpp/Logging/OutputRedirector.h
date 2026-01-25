/*  Output Redirector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Redirects a std::ostream (like std::cout or std::cerr) to a Logger.
 *  Each line written to the stream is captured and logged with a tag and color.
 */

#ifndef PokemonAutomation_Logging_OutputRedirector_H
#define PokemonAutomation_Logging_OutputRedirector_H

#include <streambuf>
#include <ostream>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Color.h"
#include "TaggedLogger.h"

namespace PokemonAutomation{


// Redirects output from a std::ostream to a Logger.
// Lines are buffered until a newline is encountered, then logged with a tag.
//
// Example usage:
//   FileLogger file_logger(config);
//   OutputRedirector cout_redirect(std::cout, file_logger, "stdout", COLOR_WHITE);
//   std::cout << "Hello World" << std::endl;  // Gets logged to file_logger
//
// The original stream buffer is restored when the OutputRedirector is destroyed.
class OutputRedirector : public std::basic_streambuf<char>{
public:
    // Construct an OutputRedirector that captures output from the given stream
    // and logs it to the given logger with the specified tag and color.
    OutputRedirector(std::ostream& stream, Logger& logger, std::string tag, Color color);
    ~OutputRedirector();

private:
    virtual int_type overflow(int_type ch) override;
    virtual std::streamsize xsputn(const char_type* s, std::streamsize count) override;

private:
    SpinLock m_lock;
    std::ostream& m_stream;
    std::streambuf* m_old_buf;
    TaggedLogger m_logger;
    Color m_color;
    std::string m_buffer;
};


}
#endif
