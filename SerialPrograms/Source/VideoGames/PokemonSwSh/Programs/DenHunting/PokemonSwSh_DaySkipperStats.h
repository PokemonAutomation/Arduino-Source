/*  Day Skipper Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DaySkipperStats_H
#define PokemonAutomation_PokemonSwSh_DaySkipperStats_H

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"

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

    virtual std::string to_str(PrintMode mode) const override{
        if (total_skips == 0){
            return StatsTracker::to_str(mode);
        }else{
            return
                "Skips Issued: " + tostr_u_commas(issued) +
                " - Skips Remaining: " + tostr_u_commas(total_skips - issued.load(std::memory_order_relaxed));
        }
    }

    std::atomic<uint64_t>& runs;
    std::atomic<uint64_t>& issued;
    uint64_t total_skips = 0;
};


}
}
}
#endif
