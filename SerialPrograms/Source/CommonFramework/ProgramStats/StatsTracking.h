/*  Stats Tracking
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StatsTracking_H
#define PokemonAutomation_StatsTracking_H

#include <vector>
#include <string>
#include <map>
#include <atomic>

namespace PokemonAutomation{

class Logger;

class StatsTracker{
    StatsTracker(const StatsTracker&) = delete;

public:
    StatsTracker() = default;
    virtual ~StatsTracker() = default;

    enum PrintMode{
        DUMP,
        DISPLAY_ON_SCREEN,
        SAVE_TO_STATS_FILE,
    };
    virtual std::string to_str(PrintMode mode) const;

    void parse_and_append_line(const std::string& line);


protected:
//    static constexpr bool HIDDEN_IF_ZERO = true;
//    static constexpr bool ALWAYS_VISIBLE = false;
    enum DisplayMode{
        ALWAYS_VISIBLE,
        HIDDEN_IF_ZERO,
        ALWAYS_HIDDEN,
    };

    struct Stat{
        std::string label;
        DisplayMode display_mode;
        Stat(std::string&& p_label, DisplayMode p_display_node = ALWAYS_VISIBLE);
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
