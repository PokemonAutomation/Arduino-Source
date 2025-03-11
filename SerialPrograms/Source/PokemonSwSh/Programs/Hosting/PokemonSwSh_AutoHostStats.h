/*  Auto-Hosting Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHostStats_H
#define PokemonAutomation_PokemonSwSh_AutoHostStats_H

#include "CommonFramework/ProgramStats/StatsTracking.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class AutoHostStats : public StatsTracker{
public:
    AutoHostStats();

    void add_raid(size_t raiders);
    void add_timeout();

private:
    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_timeouts;
    std::atomic<uint64_t>& m_empty;
    std::atomic<uint64_t>& m_full;
    std::atomic<uint64_t>& m_total;
};


}
}
}
#endif
