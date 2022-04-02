/*  Game Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GameNavigation_H
#define PokemonAutomation_PokemonBDSP_GameNavigation_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Non-Feedback

void save_game(BotBaseContext& context);

void menu_to_box(BotBaseContext& context);
void overworld_to_box(BotBaseContext& context);
void box_to_overworld(BotBaseContext& context);


//  Feedback

void overworld_to_menu(ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console);
void save_game(ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console);

void overworld_to_box(ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console);
void box_to_overworld(ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console);


}
}
}
#endif
