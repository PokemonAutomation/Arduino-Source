/*  Game Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GameNavigation_H
#define PokemonAutomation_PokemonBDSP_GameNavigation_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Non-Feedback

void save_game(ProControllerContext& context);

void menu_to_box(ProControllerContext& context);
void overworld_to_box(ProControllerContext& context);
void box_to_overworld(ProControllerContext& context);


//  Feedback

void overworld_to_menu(VideoStream& stream, ProControllerContext& context);
void save_game(VideoStream& stream, ProControllerContext& context);

void overworld_to_box(VideoStream& stream, ProControllerContext& context);
void box_to_overworld(VideoStream& stream, ProControllerContext& context);


}
}
}
#endif
