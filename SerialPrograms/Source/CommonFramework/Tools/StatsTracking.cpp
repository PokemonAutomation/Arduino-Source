/*  Stats Tracking
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/PrettyPrint.h"
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
            stats[item.first] += item.second.load(std::memory_order_relaxed);
            continue;
        }

        //  Find alias target.
        auto iter = m_stats.find(alias->second);
        if (iter != m_stats.end()){
            stats[alias->second] += item.second.load(std::memory_order_relaxed);
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



std::string stats_to_bar(
    Logger& logger,
    const StatsTracker* historical,
    const StatsTracker* current,
    const std::string& override_current
){
    std::string current_str;
    if (!override_current.empty()){
        current_str = override_current;
    }else if (current){
        current_str = current->to_str();
    }

    std::string historical_str;
    if (historical){
        historical_str = historical->to_str();
    }

    if (current_str.empty() && historical_str.empty()){
        return "";
    }

    if (!current_str.empty() && historical_str.empty()){
        logger.log(current_str);
        return current_str;
    }
    if (current_str.empty() && !historical_str.empty()){
        return "<b>Past Runs</b> - " + historical_str;
    }

    logger.log(current_str);

    std::string str;
    str += "<b>Current Run</b> - " + current_str;
    str += "<br>";
    str += "<b>Past Totals</b> - " + historical_str;

    return str;
}

























}
