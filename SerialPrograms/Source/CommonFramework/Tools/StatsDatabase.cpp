/*  Stats Database
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QSaveFile>
#include "ClientSource/Libraries/Logging.h"
#include "StatsDatabase.h"

namespace PokemonAutomation{



StatLine::StatLine(const StatsTracker& tracker)
    : m_time(current_time())
    , m_stats(tracker.to_str())
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




void StatList::operator+=(const StatsTracker& tracker){
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
StatList& StatSet::operator[](const std::string& label){
    return m_data[label];
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

void StatSet::save_to_file(const QString& filepath){
    QFile file(filepath);
    file.open(QIODevice::WriteOnly);
    std::string data = to_str();
    file.write(data.c_str(), data.size());
}
void StatSet::open_from_file(const QString& filepath){
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)){
        return;
    }

    std::string str = file.readAll().data();
    load_from_string(str.c_str());
}

bool StatSet::update_file(
    const QString& filepath,
    const std::string& label,
    const StatsTracker& tracker
){
    QFile file(filepath);
    while (!file.open(QIODevice::ReadWrite)){
        return false;
    }
    std::string data = file.readAll().data();

    StatSet set;
    set.load_from_string(data.c_str());

    set[label] += tracker;

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
//                cout << line << endl;
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

