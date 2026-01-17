/*  Last Log Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "LastLogTracker.h"

namespace PokemonAutomation{


void LastLogTracker::operator+=(std::string line){
    m_lines.emplace_back(std::move(line));
    while (m_lines.size() > m_max_lines){
        m_lines.pop_front();
    }
}

std::vector<std::string> LastLogTracker::snapshot() const{
    return std::vector<std::string>(m_lines.begin(), m_lines.end());
}


}
