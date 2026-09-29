/*  BDSP Advance Clock
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include "PokemonBDSP_AdvanceClock.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


uint64_t AdvanceClock::advance_at(WallClock time) const{
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
        time - anchor_time
    ).count();
    double advances = elapsed / tick_seconds * (double)npcs;
    double position = (double)anchor_advance + advances;
    return position <= 0 ? 0 : (uint64_t)position;
}

WallClock AdvanceClock::time_of_advance(uint64_t advance) const{
    if (npcs == 0){
        return anchor_time;
    }

    int64_t ticks = (int64_t)(advance / npcs) - (int64_t)(anchor_advance / npcs);
    return anchor_time + std::chrono::duration_cast<WallDuration>(
        std::chrono::duration<double>((double)ticks * tick_seconds)
    );
}

WallClock AdvanceClock::middle_of_advance(uint64_t advance) const{
    return time_of_advance(advance) - std::chrono::duration_cast<WallDuration>(
        std::chrono::duration<double>(tick_seconds / 2)
    );
}


}
}
}
