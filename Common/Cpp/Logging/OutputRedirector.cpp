/*  Output Redirector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "OutputRedirector.h"

namespace PokemonAutomation{


OutputRedirector::OutputRedirector(std::ostream& stream, Logger& logger, std::string tag, Color color)
    : m_stream(stream)
    , m_old_buf(stream.rdbuf())
    , m_logger(logger, std::move(tag))
    , m_color(color)
{
    stream.rdbuf(this);
}

OutputRedirector::~OutputRedirector(){
    m_stream.rdbuf(m_old_buf);
}

OutputRedirector::int_type OutputRedirector::overflow(int_type ch){
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
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
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    m_old_buf->sputn(s, count);
    m_buffer.append(s, count);
    if (!m_buffer.empty() && m_buffer.back() == '\n'){
        m_logger.log(m_buffer, m_color);
        m_buffer.clear();
    }
    return count;
}


}
