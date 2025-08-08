/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_Commands_AutoHosts.h"
//#include "PokemonSwSh_Messages_AutoHosts.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void connect_to_internet(
    ProControllerContext& context,
    Milliseconds open_ycomm_delay,
    Milliseconds connect_to_internet_delay
){
    ssf_press_button(context, BUTTON_Y, open_ycomm_delay, 80ms);

    //  Move the cursor as far away from Link Trade and Surprise Trade as possible.
    //  This is added safety in case connect to internet takes too long.
    ssf_issue_scroll(context, SSF_SCROLL_UP, 40ms, 40ms, 40ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 40ms, 40ms, 40ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 40ms, 40ms, 40ms);

    //  Connect to internet.
    ssf_press_button(context, BUTTON_PLUS, 24ms);

    //  Mash B to get out of YCOMM.
    ssf_mash1_button(context, BUTTON_B, connect_to_internet_delay);
}
void home_to_add_friends(
    ProControllerContext& context,
    uint8_t user_slot,
    uint8_t scroll_down,
    bool fix_cursor
){
//    cout << "scroll_down = " << (int)scroll_down << endl;

    //  Scroll to correct user.
    //  Do 2 up-scrolls instead of one. In the event that an error leaves you in
    //  the game instead of the Switch Home, these button presses will actually
    //  start the raid - which can kill the den. This will move the cursor over
    //  "Quit" instead of "Ready to Battle!".
    ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
    for (uint8_t c = 0; c < user_slot; c++){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    }
    ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);

    //  Enter user profile.
    ssf_press_button_ptv(context, BUTTON_A, 2000ms);

    if (fix_cursor){
        //  Force cursor to bottom, then up one to FRs.
        for (uint8_t c = 0; c < 40; c++){
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
        }
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
    }

    ssf_do_nothing(context, 50);
    ssf_issue_scroll_ptv(context, DPAD_RIGHT);
    while (scroll_down--){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
    }
}
void accept_FRs(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t slot, bool fix_cursor,
    Milliseconds game_to_home_delay_safe,
    Milliseconds auto_fr_duration,
    bool tolerate_system_update_window_slow
){
    if (slot > 7){
        slot = 7;
    }

    //  Go to Switch Home menu.
    go_home(console, context);

    home_to_add_friends(context, slot, 0, fix_cursor);

    //  Mash A.
    pbf_mash_button(context, BUTTON_A, auto_fr_duration);

    //  Return to Switch Home menu. (or game)
    if (console.video().snapshot()){
        console.log("Entering game using inference...");
        pbf_press_button(context, BUTTON_HOME, 20, 180);
        NintendoSwitch::resume_game_from_home(console, context);
    }else{
        console.log("Entering game without inference...", COLOR_RED);
        settings_to_enter_game_den_lobby(
            context,
            tolerate_system_update_window_slow, false,
            GameSettings::instance().ENTER_SWITCH_POKEMON0,
            GameSettings::instance().EXIT_SWITCH_POKEMON0
        );
    }
    pbf_wait(context, 300);
}




}
}
}


