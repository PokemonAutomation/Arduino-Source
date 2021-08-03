/*  Stats Tracking
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "ProgramEnvironment.h"
#include "StatsTracking.h"

#include <iostream>
#include <QJsonArray>
#include <QJsonObject>
using std::cout;
using std::endl;

namespace PokemonAutomation{



StatsTracker::Stat::Stat(std::string&& p_label, bool p_omit_if_zero)
    : label(p_label)
    , omit_if_zero(p_omit_if_zero)
{}
std::string StatsTracker::to_str() const{
    std::map<std::string, uint64_t> stats;
    for (const auto& item : m_stats){
        auto alias = m_aliases.find(item.first);

        //  Not an alias.
        if (alias == m_aliases.end()){
            stats[item.first] += item.second;
            continue;
        }

        //  Find alias target.
        auto iter = m_stats.find(alias->second);
        if (iter != m_stats.end()){
            stats[alias->second] += item.second;
        }
    }

    std::string str;
    for (const Stat& stat : m_display_order){
        auto iter = stats.find(stat.label);
        uint64_t count = 0;
        if (iter != stats.end()){
            count += iter->second;
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
    QString str = QString::fromStdString(to_str());
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
            if (ch < 32){
                m_stats[label] += count;
                return;
            }
            if (ch == ',') continue;
            if (!('0' <= ch && ch <= '9')) break;
            count *= 10;
            count += ch - '0';
        }

//        cout << label << " = " << count << endl;
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

QJsonArray StatsTracker::make_discord_stats() const{
    QJsonArray fields;
    for (const Stat& stat : m_display_order) {

        uint64_t count = 0;
        auto iter = m_stats.find(stat.label);
        if (iter != m_stats.end()){
            count = iter->second;
        }

        if (stat.omit_if_zero && count == 0){
            continue;
        }

        QJsonObject field;
        field["name"] = QString::fromStdString(stat.label);
        field["value"] = QString::fromStdString(tostr_u_commas(count));

        fields.append(field);
    }
    return fields;
}


}
