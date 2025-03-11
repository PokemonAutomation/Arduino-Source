/*  Output Redirector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <ostream>
#include "OutputRedirector.h"

namespace PokemonAutomation{


OutputRedirector::OutputRedirector(std::ostream& stream, std::string tag, Color color)
    : m_stream(stream)
    , m_old_buf(stream.rdbuf())
    , m_logger(global_logger_raw(), tag)
    , m_color(color)
{
    stream.rdbuf(this);
}
OutputRedirector::~OutputRedirector(){
    this->m_stream.rdbuf(this->m_old_buf);
}



OutputRedirector::int_type OutputRedirector::overflow(int_type ch){
    WriteSpinLock lg(m_lock);
    m_old_buf->sputc((char)ch);
    m_old_buf->pubsync();
    m_buffer += (char)ch;
    if (ch == '\n'){
        m_logger.log(m_buffer, m_color);
        m_buffer.clear();
    }
    return ch;
}
std::streamsize OutputRedirector::xsputn(const char_type* s, std::streamsize count){
    WriteSpinLock lg(m_lock);
    m_old_buf->sputn(s, count);
    m_buffer.append(s, count);
    if (!m_buffer.empty() && m_buffer.back() == '\n'){
        m_logger.log(m_buffer, m_color);
        m_buffer.clear();
    }
    return count;
}


}
