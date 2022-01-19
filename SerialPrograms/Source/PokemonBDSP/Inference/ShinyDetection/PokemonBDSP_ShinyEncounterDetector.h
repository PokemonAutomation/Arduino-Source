/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinyEncounterDetector_H
#define PokemonAutomation_PokemonBDSP_ShinyEncounterDetector_H

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "Pokemon/Pokemon_DataTypes.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
using namespace Pokemon;


struct DoublesShinyDetection : public ShinyDetectionResult{
    bool left_is_shiny = false;
    bool right_is_shiny = false;
};


struct DetectionType{
    ImageFloatBox box;
    PokemonSwSh::EncounterState required_state;
    std::chrono::milliseconds state_duration;
    bool full_battle_menu;
};
extern const DetectionType WILD_POKEMON;
extern const DetectionType YOUR_POKEMON;


DoublesShinyDetection detect_shiny_battle(
    Logger& logger,
    ProgramEnvironment& env,
    VideoFeed& feed, VideoOverlay& overlay,
    const DetectionType& type,
    std::chrono::seconds timeout,
    double overall_threshold = 5.0,
    double doubles_threshold = 3.0
);




}
}
}
#endif
