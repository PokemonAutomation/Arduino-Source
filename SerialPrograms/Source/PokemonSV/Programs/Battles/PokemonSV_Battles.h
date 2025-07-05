/*  Battles
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_Battles_H
#define PokemonAutomation_PokemonSV_Battles_H

#include <stdint.h>
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/Language.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "Pokemon/Pokemon_EncounterStats.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldSensors.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class ProgramEnvironment;
namespace NintendoSwitch{
    using namespace Pokemon;
namespace PokemonSV{

class EncounterBotCommonOptions;


void auto_heal_from_menu_or_overworld(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    uint8_t party_slot, //  0 - 5
    bool return_to_overworld
);



//  Returns the # of attempts it took to run.
int run_from_battle(
    VideoStream& stream, ProControllerContext& context
);
int run_from_battle(
    VideoStream& stream, ProControllerContext& context,
    OverworldBattleTracker& tracker
);



void process_battle(
    bool& caught, bool& should_save,
    ProgramEnvironment& env,
    EncounterBotCommonOptions& settings,
    VideoStream& stream, ProControllerContext& context,
    OverworldBattleTracker& tracker,
    EncounterFrequencies& encounter_frequencies,
    std::atomic<uint64_t>& shiny_counter,
    Language language
);




}
}
}
#endif
