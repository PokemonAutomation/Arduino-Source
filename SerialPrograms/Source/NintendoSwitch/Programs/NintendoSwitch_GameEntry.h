/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_GameEntry_H
#define PokemonAutomation_NintendoSwitch_GameEntry_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void go_home(ConsoleHandle& console, ProControllerContext& context);
void go_home(ConsoleHandle& console, JoyconContext& context);
void ensure_at_home(ConsoleHandle& console, ProControllerContext& context);
void ensure_at_home(ConsoleHandle& console, JoyconContext& context);



void resume_game_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    bool skip_home_press = false
);


void start_game_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    Milliseconds start_game_mash
);

bool openedgame_to_gamemenu(
    VideoStream& stream, ProControllerContext& context,
    Milliseconds timeout
);


void resume_game_from_home(
    ConsoleHandle& console, JoyconContext& context,
    bool skip_home_press = false
);
void start_game_from_home_with_inference(
    ConsoleHandle& console, JoyconContext& context,
    uint8_t game_slot,
    uint8_t user_slot,
    Milliseconds start_game_mash
);

bool openedgame_to_gamemenu(
    VideoStream& stream, JoyconContext& context,
    Milliseconds timeout
);


}
}
#endif
