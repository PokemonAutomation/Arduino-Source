/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_Commands_AutoHosts.h"
#include "PokemonSwSh_Messages_AutoHosts.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void connect_to_internet(
    const BotBaseContext& context,
    uint16_t open_ycomm_delay,
    uint16_t connect_to_internet_delay
){
    context.issue_request(
        DeviceRequest_connect_to_internet(open_ycomm_delay, connect_to_internet_delay)
    );
}
void home_to_add_friends(
    const BotBaseContext& context,
    uint8_t user_slot,
    uint8_t scroll_down,
    bool fix_cursor
){
    context.issue_request(
        DeviceRequest_home_to_add_friends(user_slot, scroll_down, fix_cursor)
    );
}
void accept_FRs(
    const BotBaseContext& context,
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
    settings_to_enter_game_den_lobby(
        context,
        tolerate_system_update_window_slow, false,
        GameSettings::instance().ENTER_SWITCH_POKEMON, GameSettings::instance().EXIT_SWITCH_POKEMON
    );
    pbf_wait(context, 300);
}



int register_message_converters_pokemon_autohosting(){
    register_message_converter(
        PABB_MSG_COMMAND_CONNECT_TO_INTERNET,
        [](const std::string& body){
            std::stringstream ss;
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
            std::stringstream ss;
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


}
}
}


