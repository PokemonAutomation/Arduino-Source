/*  Auto-Hosting Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHostStats_H
#define PokemonAutomation_PokemonSwSh_AutoHostStats_H

#include "CommonFramework/Tools/StatsTracking.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class AutoHostStats : public StatsTracker{
public:
    AutoHostStats();

    void add_raid(size_t raiders);
    void add_timeout();

private:
    uint64_t& m_raids;
    uint64_t& m_timeouts;
    uint64_t& m_empty;
    uint64_t& m_full;
    uint64_t& m_total;
};


}
}
}
#endif
