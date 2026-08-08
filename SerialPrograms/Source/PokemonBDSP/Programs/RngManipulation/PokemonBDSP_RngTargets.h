/*  BDSP RNG Targets
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_RngTargets_H
#define PokemonAutomation_PokemonBDSP_RngTargets_H

#include <stddef.h>
#include <stdint.h>
#include "Common/Cpp/Options/EnumDropdownDatabase.h"
#include "Pokemon/Pokemon_BdspRng.h"
#include "PokemonBDSP_RngTimeline.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


enum class BdspRngTarget{
    Starter,
    Dialga,
    Palkia,
    Giratina,
    Regirock,
    Regice,
    Registeel,
    Cresselia,
    TrainerId,
};


struct BdspRngTargetInfo{
    const char* slug;
    const char* display_name;
    Pokemon::BdspStaticTemplate pokemon;
    BdspTimelineContext timeline;
    uint8_t observation_npcs;
    bool has_timeline;
};


const BdspRngTargetInfo& bdsp_rng_target_info(BdspRngTarget target);

const EnumDropdownDatabase<BdspRngTarget>& BdspRngTarget_Database();


}
}
}
#endif
