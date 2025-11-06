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

    // How to print stats to string when calling to_str()
    enum PrintMode{
        DUMP,                // For logging: show all stats regardless of their DisplayMode. 
        DISPLAY_ON_SCREEN,   // For UI display: obey each stat's DisplayMode.
        SAVE_TO_STATS_FILE,  // For persisting stats to disk: skip ALWAYS_HIDDEN stats when
                             // the stat value is 0 to improve stat file readability.
    };

    // Create a string showing the stats to display on UI
    virtual std::string to_str(PrintMode mode) const;

    void parse_and_append_line(const std::string& line);


protected:
//    static constexpr bool HIDDEN_IF_ZERO = true;
//    static constexpr bool ALWAYS_VISIBLE = false;

    // How stats are displayed on the UI
    enum DisplayMode{
        ALWAYS_VISIBLE, // this stat is always visible
        HIDDEN_IF_ZERO, // if this stat value is 0, hide it from showing on UI
        ALWAYS_HIDDEN,  // always hide this stat from UI
    };

    struct Stat{
        std::string label;
        DisplayMode display_mode;
        Stat(std::string&& p_label, DisplayMode p_display_node = ALWAYS_VISIBLE);
    };

    std::vector<Stat> m_display_order;
    std::map<std::string, std::atomic<uint64_t>> m_stats;

    //  Maps old stat names to new stat names for backward compatibility.
    //  Example:
    //      // In constructor after setting up m_display_order:
    //      m_display_order.emplace_back(Stat("Red"));  // New unified stat name
    //      m_aliases["Red Detected"] = "Red";          // Old name -> new name
    //      m_aliases["Red Presumed"] = "Red";          // Another old name -> same new name
    //
    //  Counts from previous "Red Detected" and "Red Presumed" will be combined and displayed under
    //  the new name "Red" when to_str() is called.
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
