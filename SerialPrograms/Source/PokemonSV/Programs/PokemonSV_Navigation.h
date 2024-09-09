/*  Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Move between different game elements: overworld, save, internet, map, etc.
 *
 */

#ifndef PokemonAutomation_PokemonSV_Navigation_H
#define PokemonAutomation_PokemonSV_Navigation_H

//#include <stdint.h>
#include <string>

namespace PokemonAutomation{
    struct ProgramInfo;
    class ConsoleHandle;
    class BotBaseContext;
//    class ProgramEnvironment;
//    class EventNotificationOption;
namespace NintendoSwitch{
namespace PokemonSV{


void set_time_to_12am_from_home(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  Perform a No-op day skip that rolls over all the outbreaks and raids.
void day_skip_from_overworld(ConsoleHandle& console, BotBaseContext& context);

// Press B to return to the overworld
void press_Bs_to_back_to_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, uint16_t seconds_between_b_presses = 3);

//  From overworld, open map. Will change map view from rotated to fixed if not already fixed.
void open_map_from_overworld(
    const ProgramInfo& info,
    ConsoleHandle& console, 
    BotBaseContext& context,
    bool clear_tutorial = false
);

//  From map, press A to fly to a travel spot.
//  check_fly_menuitem == true: will detect if the "Fly" menuitem is available. Return false if no "Fly" menuitem (the game
//    will be on the map menu opened state). Return true if the flight is successful (the game will be at the overworld).
//  check_fly_menuitem == false: will use GradientArrowDetector to check if a map menu is opened. No "Fly" menuitem check.
//    The function always returns true. It throws an error in the case of no "Fly" menuitem. But the error message will be about
//    timeout running the function.
bool fly_to_overworld_from_map(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, bool check_fly_menuitem = false);

//  Assume the user can set up picnic at current location, start picnic from overworld.
void picnic_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  While in picnic, stop picnic and back to overworld.
void leave_picnic(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  Enter box system from overworld.
void enter_box_system_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  From box system go to overworld.
void leave_box_system_to_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  From overworld, open Pokédex.
void open_pokedex_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  From Pokédex, open Recently Battled.
void open_recently_battled_from_pokedex(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  From any of the rotom phone apps (Map/Pokédex/Profile) go to overworld.
void leave_phone_to_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  While on map (default zoom), move to the closest PokeCenter and fly there.
//  The PokeCenter must be already visited before (so having the little wing icon with it) and not occluded
//  by other map icons on the most zoomed-in level of the map.
void fly_to_closest_pokecenter_on_map(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

// Attempt to escape being stuck on a wall.
// Repeatedly center camera and try to backwards jump off.
// Finishes when map is successfully open.
void jump_off_wall_until_map_open(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

void reset_to_pokecenter(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

}
}
}
#endif
