/*  Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Move between different game elements: overworld, save, internet, map, etc.
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


//  Save game from menu.
//  Will throw OperationFailedException.
void save_game_from_menu(ConsoleHandle& console, BotBaseContext& context);
//  Save game from overworld.
//  Will throw OperationFailedException.
void save_game_from_overworld(ConsoleHandle& console, BotBaseContext& context);

void connect_to_internet_from_overworld(ConsoleHandle& console, BotBaseContext& context);

void set_time_to_12am_from_home(ConsoleHandle& console, BotBaseContext& context);

//  Perform a No-op day skip that rolls over all the outbreaks and raids.
void day_skip_from_overworld(ConsoleHandle& console, BotBaseContext& context);

//  From overworld, open map. Will change map view from rotated to fixed if not already fixed.
//  Will throw OperationFailedException.
void open_map_from_overworld(ConsoleHandle& console, BotBaseContext& context);

//  From map, press A to fly to a travel spot.
//  Will throw OperationFailedException.
void fly_to_overworld_from_map(ConsoleHandle& console, BotBaseContext& context);

//  Assume the user can set up picnic at current location, start picnic from overworld.
//  Will throw OperationFailedException.
void picnic_from_overworld(ConsoleHandle& console, BotBaseContext& context);

//  While in picnic, stop picnic and back to overworld.
void leave_picnic(ConsoleHandle& console, BotBaseContext& context);

}
}
}
#endif
