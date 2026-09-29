/*  BDSP Advance Clock
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AdvanceClock_H
#define PokemonAutomation_PokemonBDSP_AdvanceClock_H

#include <stdint.h>
#include "Common/Cpp/Time.h"
#include "PokemonBDSP_BlinkModel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


struct AdvanceClock{
    WallClock anchor_time{};
    uint64_t anchor_advance = 0;
    double tick_seconds = BDSP_NPC_TICK_SECONDS;
    uint8_t npcs = 1;

    uint64_t advance_at(WallClock time) const;


    //  the tick's leading edge, not necessarily safe to aim at
    WallClock time_of_advance(uint64_t advance) const;

    //  half a tick earlier
    WallClock middle_of_advance(uint64_t advance) const;
};


}
}
}
#endif
