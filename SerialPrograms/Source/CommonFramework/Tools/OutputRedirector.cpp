/*  Output Redirector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "OutputRedirector.h"

namespace PokemonAutomation{


OutputRedirector::OutputRedirector(std::ostream& stream, std::string tag)
    : m_stream(stream)
    , m_old_buf(stream.rdbuf())
    , m_logger(global_logger_raw(), tag)
{
    stream.rdbuf(this);
}
OutputRedirector::~OutputRedirector(){
    this->m_stream.rdbuf(this->m_old_buf);
}



OutputRedirector::int_type OutputRedirector::overflow(int_type ch){
    m_old_buf->sputc('\n');
    m_old_buf->pubsync();
    return ch;
}
std::streamsize OutputRedirector::xsputn(const char_type* s, std::streamsize count){
    m_old_buf->sputn(s, count);
    m_logger.log(std::string(s, count));
    return count;
}


}
