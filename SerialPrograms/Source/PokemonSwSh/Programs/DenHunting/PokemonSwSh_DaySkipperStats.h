/*  Day Skipper Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DaySkipperStats_H
#define PokemonAutomation_PokemonSwSh_DaySkipperStats_H

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Tools/StatsTracking.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class SkipperStats : public StatsTracker{
public:
    SkipperStats()
        : runs(m_stats["Runs"])
        , issued(m_stats["Skips Issued"])
    {
        m_display_order.emplace_back(Stat("Runs"));
        m_display_order.emplace_back(Stat("Skips Issued"));
    }

    std::string to_str_current(uint64_t skips_remaining) const{
        return
            "Skips Issued: " + std::to_string(issued) +
            " - Skips Remaining: " + tostr_u_commas(skips_remaining);
    }

    std::atomic<uint64_t>& runs;
    std::atomic<uint64_t>& issued;
};


}
}
}
#endif
