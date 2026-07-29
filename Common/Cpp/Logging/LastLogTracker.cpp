/*  Last Log Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include <cstring>
#include "LastLogTracker.h"

namespace PokemonAutomation{


void LastLogTracker::log(const std::string& msg, Color color){
    WriteSpinLock lg(m_lock);
    m_lines.emplace_back(color, msg);
    while (m_lines.size() > m_max_lines){
        m_lines.pop_front();
    }
}
void LastLogTracker::log(std::string&& msg, Color color){
    WriteSpinLock lg(m_lock);
    m_lines.emplace_back(color, std::move(msg));
    while (m_lines.size() > m_max_lines){
        m_lines.pop_front();
    }
}
void LastLogTracker::log(const char* msg, Color color){
    WriteSpinLock lg(m_lock);
    m_lines.emplace_back(color, msg);
    while (m_lines.size() > m_max_lines){
        m_lines.pop_front();
    }
}

std::vector<LogLine> LastLogTracker::get_recent(size_t lines) const{
    ReadSpinLock lg(m_lock);
    if (lines < m_lines.size()){
        return std::vector<LogLine>(m_lines.begin(), m_lines.begin() + lines);
    }
    return std::vector<LogLine>(m_lines.begin(), m_lines.end());
}


}
