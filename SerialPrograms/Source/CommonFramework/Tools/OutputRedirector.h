/*  Output Redirector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OutputRedirector_H
#define PokemonAutomation_OutputRedirector_H

#include <streambuf>
#include "Logger.h"

namespace PokemonAutomation{


class OutputRedirector : public std::basic_streambuf<char>{
public:
    OutputRedirector(std::ostream& stream, std::string tag);
    ~OutputRedirector();

private:
    virtual int_type overflow(int_type ch) override;
    virtual std::streamsize xsputn(const char_type* s, std::streamsize count) override;

private:
    std::ostream& m_stream;
    std::streambuf* m_old_buf;
    TaggedLogger m_logger;
};


}
#endif
