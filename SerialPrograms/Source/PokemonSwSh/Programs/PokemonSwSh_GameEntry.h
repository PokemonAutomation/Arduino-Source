/*  Start Game
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StartGame_H
#define PokemonAutomation_PokemonSwSh_StartGame_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void resume_game_no_interact(
    VideoStream& stream, ProControllerContext& context,
    bool tolerate_update_menu
);
void resume_game_back_out(
    VideoStream& stream, ProControllerContext& context,
    bool tolerate_update_menu, uint16_t mash_B_time
);



//  Start the game with the specified "game_slot" and "user_slot".
//  If "game_slot" is zero, it uses whatever the cursor is on.
//  If "user_slot" is zero, it uses whatever the cursor is on.
void start_game_from_home_with_inference(
    VideoStream& stream, ProControllerContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot = 0,
    uint8_t user_slot = 0,
    bool backup_save = false,
    uint16_t post_wait_time = 1 * TICKS_PER_SECOND
);

void reset_game_from_home_with_inference(
    VideoStream& stream, ProControllerContext& context,
    bool tolerate_update_menu,
    bool backup_save = false,
    uint16_t post_wait_time = 1 * TICKS_PER_SECOND
);

void start_game_from_home(
    VideoStream& stream, ProControllerContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot = 0,
    uint8_t user_slot = 0,
    bool backup_save = false,
    uint16_t post_wait_time = 1 * TICKS_PER_SECOND
);



}
}
}
#endif
