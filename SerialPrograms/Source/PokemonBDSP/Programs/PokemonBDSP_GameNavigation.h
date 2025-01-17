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

void save_game(ControllerContext& context);

void menu_to_box(ControllerContext& context);
void overworld_to_box(ControllerContext& context);
void box_to_overworld(ControllerContext& context);


//  Feedback

void overworld_to_menu(ConsoleHandle& console, ControllerContext& context);
void save_game(ConsoleHandle& console, ControllerContext& context);

void overworld_to_box(ConsoleHandle& console, ControllerContext& context);
void box_to_overworld(ConsoleHandle& console, ControllerContext& context);


}
}
}
#endif
