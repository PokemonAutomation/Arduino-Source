/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GameEntry_H
#define PokemonAutomation_PokemonBDSP_GameEntry_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace std::chrono_literals;


bool reset_game_from_home(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu,
    Milliseconds post_wait_time = 1000ms
);



}
}
}
#endif
