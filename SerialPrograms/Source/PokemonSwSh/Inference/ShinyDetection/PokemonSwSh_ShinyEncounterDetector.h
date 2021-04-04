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

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

enum class ShinyDetection{
    NO_BATTLE_MENU,
    NOT_SHINY,
    STAR_SHINY,
    SQUARE_SHINY,
    UNKNOWN_SHINY,
};

struct ShinyDetectionBattle{
    InferenceBox detection_box;
    std::chrono::milliseconds dialog_delay_when_shiny;
};
extern const ShinyDetectionBattle SHINY_BATTLE_REGULAR;
extern const ShinyDetectionBattle SHINY_BATTLE_RAID;



ShinyDetection detect_shiny_battle(
    ProgramEnvironment& env,
    VideoFeed& feed, Logger& logger,
    const ShinyDetectionBattle& battle_settings,
    std::chrono::seconds timeout,
    double detection_threshold = 2.0
);




}
}
}
#endif
