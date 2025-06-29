/*  Stats Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StatsDatabase_H
#define PokemonAutomation_StatsDatabase_H

#include "StatsTracking.h"

namespace PokemonAutomation{


class StatLine{
public:
    StatLine(StatsTracker& tracker);
    StatLine(const std::string& line);

    const std::string& stats() const{ return m_stats; }
    std::string to_str() const;

private:
    std::string m_time;
    std::string m_stats;
};



class StatList{
public:
    void operator+=(StatsTracker& tracker);
    void operator+=(const std::string& line);

    size_t size() const{ return m_list.size(); }
    std::string to_str() const;

    const std::vector<StatLine>& list() const{ return m_list; }

    void aggregate(StatsTracker& tracker) const;

private:
    std::vector<StatLine> m_list;
};




class StatSet{
public:
//    StatList* find(const std::string& label);
    StatList& operator[](const std::string& identifier);

    std::string to_str() const;

    void save_to_file(const std::string& filepath);
    void open_from_file(const std::string& filepath);

    static bool update_file(
        const std::string& filepath,
        const std::string& identifier,
        StatsTracker& tracker
    );

private:
    bool get_line(std::string& line, const char*& ptr);
    void load_from_string(const char* ptr);

private:
    std::map<std::string, StatList> m_data;
};



}
#endif
