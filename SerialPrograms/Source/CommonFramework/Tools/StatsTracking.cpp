/*  Stats Tracking
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "ProgramEnvironment.h"
#include "StatsTracking.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



StatsTracker::Stat::Stat(std::string&& p_label, bool p_omit_if_zero)
    : label(p_label)
    , omit_if_zero(p_omit_if_zero)
{}
std::string StatsTracker::to_str() const{
    std::string str;
    for (const Stat& stat : m_display_order){
        auto iter = m_stats.find(stat.label);
        uint64_t count = 0;
        if (iter != m_stats.end()){
            count = iter->second;
        }
        if (stat.omit_if_zero && count == 0){
            continue;
        }
        if (!str.empty()){
            str += " - ";
        }
        str += stat.label;
        str += ": ";
        str += tostr_u_commas(count);
    }
    return str;
}

#if 0
void StatsTracker::log_stats(ProgramEnvironment& env) const{
    QString str = QString(to_str().c_str());
    env.set_status(str);
    env.log(str);
}
#endif



void StatsTracker::parse_and_append_line(const std::string& line){
    const char* ptr = line.c_str();
    while (true){
        //  Parse label.
        std::string label;
        while (true){
            char ch = *ptr++;
            if (ch < 32) return;
            if (ch == ':') break;
            label += ch;
        }

        //  Skip to number.
        while (true){
            char ch = *ptr;
            if (ch < 32) return;
            if ('0' <= ch && ch <= '9') break;
            ptr++;
        }

        //  Parse number.
        uint64_t count = 0;
        while (true){
            char ch = *ptr++;
            if (ch < 32) break;
            if (ch == ',') continue;
            if (!('0' <= ch && ch <= '9')) break;
            count *= 10;
            count += ch - '0';
        }

        cout << label << " = " << count << endl;
        m_stats[label] += count;

        //  Skip to next;
        while (true){
            char ch = *ptr;
            if (ch < 32) return;
            if (('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z')) break;
            ptr++;
        }
    }
}


}
