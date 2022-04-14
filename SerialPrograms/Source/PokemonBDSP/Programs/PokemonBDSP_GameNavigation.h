/*  Game Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GameNavigation_H
#define PokemonAutomation_PokemonBDSP_GameNavigation_H

#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Non-Feedback

void save_game(BotBaseContext& context);

void menu_to_box(BotBaseContext& context);
void overworld_to_box(BotBaseContext& context);
void box_to_overworld(BotBaseContext& context);


//  Feedback

void overworld_to_menu(ConsoleHandle& console, BotBaseContext& context);
void save_game(ConsoleHandle& console, BotBaseContext& context);

void overworld_to_box(ConsoleHandle& console, BotBaseContext& context);
void box_to_overworld(ConsoleHandle& console, BotBaseContext& context);


}
}
}
#endif
