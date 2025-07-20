/*  Stats Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFile>
#include <QSaveFile>
#include "ClientSource/Libraries/Logging.h"
#include "StatsDatabase.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

const std::map<std::string, std::string> STATS_DATABASE_ALIASES{
    {"Dex Rec Finder", "PokemonSwSh:DexRecFinder"},
    {"Day Skipper (JPN)", "PokemonSwSh:DaySkipperJPN"},
    {"Day Skipper (EU)", "PokemonSwSh:DaySkipperEU"},
    {"Day Skipper (US)", "PokemonSwSh:DaySkipperUS"},
    {"Day Skipper (JPN) - 7.8k", "PokemonSwSh:DaySkipperJPN7p8k"},
    {"Purple Beam Finder", "PokemonSwSh:PurpleBeamFinder"},
    {"Auto-Host Multi-Game", "PokemonSwSh:AutoHostMultiGame"},
    {"Auto-Host Rolling", "PokemonSwSh:AutoHostRolling"},
    {"Stats Reset", "PokemonSwSh:StatsReset"},
    {"Shiny Hunt Autonomous - Regi", "PokemonSwSh:ShinyHuntAutonomousRegi"},
    {"Shiny Hunt Autonomous - Swords Of Justice", "PokemonSwSh:ShinyHuntAutonomousSwordsOfJustice"},
    {"Shiny Hunt Autonomous - Strong Spawn", "PokemonSwSh:ShinyHuntAutonomousStrongSpawn"},
    {"Shiny Hunt Autonomous - Regigigas2", "PokemonSwSh:ShinyHuntAutonomousRegigigas2"},
    {"Shiny Hunt Autonomous - IoA Trade", "PokemonSwSh:ShinyHuntAutonomousIoATrade"},
    {"Shiny Hunt Autonomous - Berry Tree", "PokemonSwSh:ShinyHuntAutonomousBerryTree"},
    {"Shiny Hunt Autonomous - Whistling", "PokemonSwSh:ShinyHuntAutonomousWhistling"},
    {"Shiny Hunt Autonomous - Fishing", "PokemonSwSh:ShinyHuntAutonomousFishing"},
    {"Shiny Hunt Autonomous - Overworld", "PokemonSwSh:ShinyHuntAutonomousOverworld"},
    {"PokemonSV:StatsResetBloodmoon", "PokemonSV:StatsResetEventBattle"},
};



StatLine::StatLine(StatsTracker& tracker)
    : m_time(current_time_to_str())
    , m_stats(tracker.to_str(StatsTracker::SAVE_TO_STATS_FILE))
{}
StatLine::StatLine(const std::string& line){
    size_t pos = line.find(" - ");
    if (pos == std::string::npos){
        m_time = line;
        return;
    }
    m_time = line.substr(0, pos);

    const char* ptr = line.c_str() + pos + 3;

    //  Skip to stats.
    while (true){
        char ch = *ptr;
        if (ch < 32) return;
        if (('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z')) break;
        ptr++;
    }

    m_stats = ptr;
}
std::string StatLine::to_str() const{
    return m_time + " - " + m_stats;
#if 0
    std::string str = m_time + " - ";
    str += m_tracker
        ? m_tracker->to_str()
        : m_stats;
    return m_time + " - " +
        (m_tracker ? m_tracker->to_str() : m_stats);
#endif
}




void StatList::operator+=(StatsTracker& tracker){
    m_list.emplace_back(tracker);
}
void StatList::operator+=(const std::string& line){
    m_list.emplace_back(line);
}
std::string StatList::to_str() const{
    std::string str;
    for (const StatLine& line : m_list){
        str += line.to_str();
        str += "\r\n";
    }
    return str;
}

void StatList::aggregate(StatsTracker& tracker) const{
    for (const StatLine& line : m_list){
        tracker.parse_and_append_line(line.stats());
//        cout << tracker.to_str() << endl;
    }
}




#if 0
StatList* StatSet::find(const std::string& label){
    auto iter = m_data.find(label);
    return iter == m_data.end()
        ? nullptr
        : &iter->second;
}
#endif
StatList& StatSet::operator[](const std::string& identifier){
    return m_data[identifier];
}

std::string StatSet::to_str() const{
    std::string str;
    for (const auto& item : m_data){
        if (item.second.size() == 0){
            continue;
        }
        str += "================================================================================\r\n";
        str += item.first;
        str += "\r\n";
        str += "\r\n";
        str += item.second.to_str();
        str += "\r\n";
    }
    return str;
}

void StatSet::save_to_file(const std::string& filepath){
    QFile file(QString::fromStdString(filepath));
    file.open(QIODevice::WriteOnly);
    std::string data = to_str();
    file.write(data.c_str(), data.size());
}
void StatSet::open_from_file(const std::string& filepath){
    QFile file(QString::fromStdString(filepath));
    if (!file.open(QIODevice::ReadOnly)){
        return;
    }

    std::string str = file.readAll().data();
    load_from_string(str.c_str());
}

bool StatSet::update_file(
    const std::string& filepath,
    const std::string& identifier,
    StatsTracker& tracker
){
    QFile file(QString::fromStdString(filepath));
    while (!file.open(QIODevice::ReadWrite)){
        return false;
    }
    std::string data = file.readAll().data();

    StatSet set;
    set.load_from_string(data.c_str());

    set[identifier] += tracker;

    data = set.to_str();
    file.seek(0);
    file.write(data.c_str(), data.size());
    file.resize(data.size());

    return true;
}


bool StatSet::get_line(std::string& line, const char*& ptr){
    line.clear();
    for (;; ptr++){
        char ch = *ptr;
        if (ch == '\0'){
//            cout << line << endl;
            ptr++;
            return false;
        }
        if (ch == '\r'){
            continue;
        }
        if (ch == '\n'){
//            cout << line << endl;
            ptr++;
            return true;
        }
        line += ch;
    }
}
void StatSet::load_from_string(const char* ptr){
    m_data.clear();

    //  Find first section.
    while (true){
        std::string line;
        if (!get_line(line, ptr)){
            return;
        }
        if (line[0] == '='){
            break;
        }
    }

    while (true){
        std::string line;
        if (!get_line(line, ptr)){
            return;
        }
        if (line.empty()){
            continue;
        }

        auto iter = STATS_DATABASE_ALIASES.find(line);
        if (iter != STATS_DATABASE_ALIASES.end()){
            line = iter->second;
        }

        StatList& program = m_data[line];
        while (true){
            if (!get_line(line, ptr)){
                return;
            }
            if (line.empty()){
                continue;
            }
            if (line[0] == '='){
                break;
            }
            program += line;
        }
    }
}









}

