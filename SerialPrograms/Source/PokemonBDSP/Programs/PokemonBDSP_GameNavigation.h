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

void save_game(const BotBaseContext& context);

void menu_to_box(const BotBaseContext& context);
void overworld_to_box(const BotBaseContext& context);
void box_to_overworld(const BotBaseContext& context);


//  Feedback

void overworld_to_menu(ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console);
void save_game(ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console);

void overworld_to_box(ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console);
void box_to_overworld(ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console);


}
}
}
#endif
