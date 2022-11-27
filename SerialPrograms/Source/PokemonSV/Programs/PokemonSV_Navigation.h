/*  Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_Navigation_H
#define PokemonAutomation_PokemonSV_Navigation_H

//#include <stdint.h>
#include <string>

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
//    class ProgramEnvironment;
//    class EventNotificationOption;
namespace NintendoSwitch{
namespace PokemonSV{



void save_game_from_menu(ConsoleHandle& console, BotBaseContext& context);
void save_game_from_overworld(ConsoleHandle& console, BotBaseContext& context);

void connect_to_internet_from_overworld(ConsoleHandle& console, BotBaseContext& context);

void set_time_to_12am_from_home(ConsoleHandle& console, BotBaseContext& context);





}
}
}
#endif
