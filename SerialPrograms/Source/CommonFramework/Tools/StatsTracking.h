/*  Stats Tracking
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_StatsTracking_H
#define PokemonAutomation_StatsTracking_H

#include <vector>
#include <string>
#include <map>
#include <atomic>

class QJsonArray;

namespace PokemonAutomation{

class Logger;

class StatsTracker{
    StatsTracker(const StatsTracker&) = delete;

public:
    StatsTracker() = default;
    virtual ~StatsTracker() = default;

    std::string to_str() const;

    void parse_and_append_line(const std::string& line);


protected:
    struct Stat{
        std::string label;
        bool omit_if_zero;
        Stat(std::string&& p_label, bool p_omit_if_zero = false);
    };

    std::vector<Stat> m_display_order;
    std::map<std::string, std::atomic<uint64_t>> m_stats;
    std::map<std::string, std::string> m_aliases;
};


std::string stats_to_bar(
    Logger& logger,
    const StatsTracker* historical,
    const StatsTracker* current,
    const std::string& override_current = ""
);




}
#endif
