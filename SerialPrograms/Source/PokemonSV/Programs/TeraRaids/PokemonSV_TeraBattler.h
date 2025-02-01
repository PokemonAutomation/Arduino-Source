/*  Tera Battler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraBattler_H
#define PokemonAutomation_PokemonSV_TeraBattler_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "PokemonSV/Options/PokemonSV_TeraAIOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//  Run a tera battle until you either win or lose.
bool run_tera_battle(
    ProgramEnvironment& env,
    VideoStream& stream, SwitchControllerContext& context,
    TeraAIOption& battle_AI
);




}
}
}
#endif
