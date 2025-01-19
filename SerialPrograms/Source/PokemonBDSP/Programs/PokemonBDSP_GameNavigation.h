/*  Game Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GameNavigation_H
#define PokemonAutomation_PokemonBDSP_GameNavigation_H

#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Non-Feedback

void save_game(SwitchControllerContext& context);

void menu_to_box(SwitchControllerContext& context);
void overworld_to_box(SwitchControllerContext& context);
void box_to_overworld(SwitchControllerContext& context);


//  Feedback

void overworld_to_menu(ConsoleHandle& console, SwitchControllerContext& context);
void save_game(ConsoleHandle& console, SwitchControllerContext& context);

void overworld_to_box(ConsoleHandle& console, SwitchControllerContext& context);
void box_to_overworld(ConsoleHandle& console, SwitchControllerContext& context);


}
}
}
#endif
