/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_Commands_AutoHosts.h"
//#include "PokemonSwSh_Messages_AutoHosts.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void connect_to_internet(
    ControllerContext& context,
    uint16_t open_ycomm_delay,
    uint16_t connect_to_internet_delay
){
#if 0
    context.issue_request(
        DeviceRequest_connect_to_internet(open_ycomm_delay, connect_to_internet_delay)
    );
#else
    ssf_press_button(context, BUTTON_Y, open_ycomm_delay, 10);

    //  Move the cursor as far away from Link Trade and Surprise Trade as possible.
    //  This is added safety in case connect to internet takes too long.
    ssf_issue_scroll(context, SSF_SCROLL_UP, 5);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 5);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 5);

    //  Connect to internet.
    ssf_press_button(context, BUTTON_PLUS, 3);

    //  Mash B to get out of YCOMM.
    ssf_mash1_button(context, BUTTON_B, connect_to_internet_delay);
#endif
}
void home_to_add_friends(
    ControllerContext& context,
    uint8_t user_slot,
    uint8_t scroll_down,
    bool fix_cursor
){
#if 0
    context.issue_request(
        DeviceRequest_home_to_add_friends(user_slot, scroll_down, fix_cursor)
    );
#else
    //  Scroll to correct user.
    //  Do 2 up-scrolls instead of one. In the event that an error leaves you in
    //  the game instead of the Switch Home, these button presses will actually
    //  start the raid - which can kill the den. This will move the cursor over
    //  "Quit" instead of "Ready to Battle!".
    ssf_issue_scroll(context, SSF_SCROLL_UP, 3);
    for (uint8_t c = 0; c < user_slot; c++){
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    }
    ssf_issue_scroll(context, SSF_SCROLL_UP, 3);

    //  Enter user profile.
    ssf_press_button(context, BUTTON_A, 200);

    if (fix_cursor){
        //  Force cursor to bottom, then up one to FRs.
        for (uint8_t c = 0; c < 20; c++){
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
        }
        ssf_issue_scroll(context, SSF_SCROLL_UP, 3);
    }

    ssf_do_nothing(context, 50);
    ssf_press_dpad(context, DPAD_RIGHT, 3);
    while (scroll_down--){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
    }
#endif
}
void accept_FRs(
    ConsoleHandle& console, ControllerContext& context,
    uint8_t slot, bool fix_cursor,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
){
    if (slot > 7){
        slot = 7;
    }

    //  Go to Switch Home menu.
    pbf_press_button(context, BUTTON_HOME, 10, game_to_home_delay_safe);

    home_to_add_friends(context, slot, 0, fix_cursor);

    //  Mash A.
    pbf_mash_button(context, BUTTON_A, auto_fr_duration);

    //  Return to Switch Home menu. (or game)
    if (console.video().snapshot()){
        console.log("Entering game using inference...");
        pbf_press_button(context, BUTTON_HOME, 10, 190);
        NintendoSwitch::resume_game_from_home(console, context);
    }else{
        console.log("Entering game without inference...", COLOR_RED);
        settings_to_enter_game_den_lobby(
            context,
            tolerate_system_update_window_slow, false,
            GameSettings::instance().ENTER_SWITCH_POKEMON, GameSettings::instance().EXIT_SWITCH_POKEMON
        );
    }
    pbf_wait(context, 300);
}



#if 0
int register_message_converters_pokemon_autohosting(){
    register_message_converter(
        PABB_MSG_COMMAND_CONNECT_TO_INTERNET,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "connect_to_internet() - ";
            if (body.size() != sizeof(pabb_connect_to_internet)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_connect_to_internet*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", open_ycomm_delay = " << params->open_ycomm_delay;
            ss << ", connect_to_internet_delay = " << params->connect_to_internet_delay;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_HOME_TO_ADD_FRIENDS,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "home_to_add_friends() - ";
            if (body.size() != sizeof(pabb_home_to_add_friends)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_home_to_add_friends*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", user_slot = " << (unsigned)params->user_slot;
            ss << ", fix_cursor = " << params->fix_cursor;
            return ss.str();
        }
    );
    return 0;
}
int init_PokemonSwShAutoHosts = register_message_converters_pokemon_autohosting();
#endif


}
}
}


