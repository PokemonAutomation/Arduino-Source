/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyEncounterDetector_H
#define PokemonAutomation_PokemonSwSh_ShinyEncounterDetector_H

#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


struct ShinyDetectionBattle{
    InferenceBox detection_box;
    std::chrono::milliseconds dialog_delay_when_shiny;
};
extern const ShinyDetectionBattle SHINY_BATTLE_REGULAR;
extern const ShinyDetectionBattle SHINY_BATTLE_RAID;



ShinyType detect_shiny_battle(
    ProgramEnvironment& env, VideoFeed& feed,
    const ShinyDetectionBattle& battle_settings,
    std::chrono::seconds timeout,
    double detection_threshold = 2.0
);




}
}
}
#endif
