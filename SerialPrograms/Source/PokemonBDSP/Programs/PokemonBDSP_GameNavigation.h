/*  Game Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GameNavigation_H
#define PokemonAutomation_PokemonBDSP_GameNavigation_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Non-Feedback

void save_game(SwitchControllerContext& context);

void menu_to_box(SwitchControllerContext& context);
void overworld_to_box(SwitchControllerContext& context);
void box_to_overworld(SwitchControllerContext& context);


//  Feedback

void overworld_to_menu(VideoStream& stream, SwitchControllerContext& context);
void save_game(VideoStream& stream, SwitchControllerContext& context);

void overworld_to_box(VideoStream& stream, SwitchControllerContext& context);
void box_to_overworld(VideoStream& stream, SwitchControllerContext& context);


}
}
}
#endif
