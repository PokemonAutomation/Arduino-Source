/*  BDSP RNG Timeline
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_RngTimeline_H
#define PokemonAutomation_PokemonBDSP_RngTimeline_H

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include "Common/Cpp/Time.h"
#include "Pokemon/Pokemon_Xorshift128.h"
#include "PokemonBDSP_BlinkModel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


struct BdspTimelineContext{
    uint8_t npcs = 1;
    uint8_t pokemon_models = 0;
    double white_delay_seconds = 0;
    uint32_t advance_delay = 0;
    uint32_t advance_delay_2 = 0;
    uint32_t advance_delay_2_after_events = 10;
    bool plus_one_on_menu_close = false;
    uint32_t advances_after_accept = 0;
};


struct BdspTimelineResult{
    std::vector<double> advance_times;
    Pokemon::Xorshift128State end_state;
    bool reached_target = false;
    uint64_t advances() const{ return advance_times.size(); }
    double duration_seconds() const{
        return advance_times.empty() ? 0.0 : advance_times.back();
    }
};


BdspTimelineResult simulate_timeline(
    Pokemon::Xorshift128 rng,
    const BdspTimelineContext& context,
    uint64_t max_advances
);

bool time_of_advance(const BdspTimelineResult& result, uint64_t advance, double& seconds);


struct AdvanceClock{
    WallClock anchor_time{};
    uint64_t anchor_advance = 0;
    double tick_seconds = BDSP_NPC_TICK_SECONDS;
    uint8_t npcs = 1;

    uint64_t advance_at(WallClock time) const;


    //  the tick's leading edge, not necessarily safe to aim at
    WallClock time_of_advance(uint64_t advance) const;

    //  half a tick later
    WallClock middle_of_advance(uint64_t advance) const;
};


}
}
}
#endif
