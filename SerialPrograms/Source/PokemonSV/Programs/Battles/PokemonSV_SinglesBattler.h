/*  Singles Battler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_SinglesBattler_H
#define PokemonAutomation_PokemonSV_SinglesBattler_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "PokemonSV/Options/PokemonSV_SinglesMoveTable.h"
#include "PokemonSV/Options/PokemonSV_SinglesAIOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//enum RunPokemonResult{
//    OWN_FAINTED,
//    OVERWORLD,
//};

bool run_pokemon(
    VideoStream& stream, SwitchControllerContext& context,
    const std::vector<SinglesMoveEntry>& move_table,
    bool trainer_battle, bool& terastallized
);


//  Run a singles battle until it is over (for whatever reason).
bool run_singles_battle(
    ProgramEnvironment& env,
    VideoStream& stream, SwitchControllerContext& context,
    SinglesAIOption& battle_AI,
    bool trainer_battle
);



}
}
}
#endif
