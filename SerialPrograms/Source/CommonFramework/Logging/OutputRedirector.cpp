/*  Output Redirector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <ostream>
#include "OutputRedirector.h"

namespace PokemonAutomation{


OutputRedirector::OutputRedirector(std::ostream& stream, std::string tag, QColor color)
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
    SpinLockGuard lg(m_lock);
    m_old_buf->sputc(ch);
    m_old_buf->pubsync();
    m_buffer += ch;
    if (ch == '\n'){
        m_logger.log(m_buffer, m_color);
        m_buffer.clear();
    }
    return ch;
}
std::streamsize OutputRedirector::xsputn(const char_type* s, std::streamsize count){
    SpinLockGuard lg(m_lock);
    m_old_buf->sputn(s, count);
    m_buffer.append(s, count);
    if (!m_buffer.empty() && m_buffer.back() == '\n'){
        m_logger.log(m_buffer, m_color);
        m_buffer.clear();
    }
    return count;
}


}
