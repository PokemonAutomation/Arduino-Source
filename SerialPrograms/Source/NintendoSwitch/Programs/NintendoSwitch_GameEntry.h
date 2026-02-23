/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_GameEntry_H
#define PokemonAutomation_NintendoSwitch_GameEntry_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


// Press Home button to go from game to Switch Home screen
void go_home(ConsoleHandle& console, ProControllerContext& context);
// Press Home button to go from game to Switch Home screen
void go_home(ConsoleHandle& console, JoyconContext& context);
void ensure_at_home(ConsoleHandle& console, ProControllerContext& context);
void ensure_at_home(ConsoleHandle& console, JoyconContext& context);

void close_game_from_home(ConsoleHandle& console, ProControllerContext& device);
void close_game_from_home(ConsoleHandle& console, JoyconContext& device);

void resume_game_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    bool skip_home_press = false
);
void resume_game_from_home(
    ConsoleHandle& console, JoyconContext& context,
    bool skip_home_press = false
);

void start_game_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot
);
void start_game_from_home(
    ConsoleHandle& console, JoyconContext& context,
    uint8_t game_slot,
    uint8_t user_slot
);

//  From the home menu:
//      1.  Close the game.
//      2.  Restart the game using the current user.
//  This does not enter all the way into the game. It only starts the reopens
//  the game.
void from_home_close_and_reopen_game(
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu
);


bool openedgame_to_gamemenu(
    VideoStream& stream, ProControllerContext& context,
    Milliseconds timeout
);
bool openedgame_to_gamemenu(
    VideoStream& stream, JoyconContext& context,
    Milliseconds timeout
);


}
}
#endif
