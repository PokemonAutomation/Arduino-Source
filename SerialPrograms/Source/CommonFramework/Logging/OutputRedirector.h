/*  Output Redirector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Logging_OutputRedirector_H
#define PokemonAutomation_Logging_OutputRedirector_H

#include <streambuf>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Logger.h"

namespace PokemonAutomation{


class OutputRedirector : public std::basic_streambuf<char>{
public:
    OutputRedirector(std::ostream& stream, std::string tag, Color color);
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
