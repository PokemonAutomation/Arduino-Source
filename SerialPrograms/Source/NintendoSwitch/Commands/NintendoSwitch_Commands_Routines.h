/*  General Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_Routines_H
#define PokemonAutomation_NintendoSwitch_Commands_Routines_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void close_game_from_home(ConsoleHandle& console, ProControllerContext& device);

void close_game_from_home(ConsoleHandle& console, JoyconContext& device);




}
}
#endif
