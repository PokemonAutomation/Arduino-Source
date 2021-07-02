/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwShAutoHosts.h"


#if 0
void connect_to_internet(uint16_t open_ycomm_delay, uint16_t connect_to_internet_delay){
    connect_to_internet(*PokemonAutomation::global_connection, open_ycomm_delay, connect_to_internet_delay);
}
void home_to_add_friends(uint8_t user_slot, uint8_t scroll_down, bool fix_cursor){
    home_to_add_friends(*PokemonAutomation::global_connection, user_slot, scroll_down, fix_cursor);
}
uint16_t accept_FRs(
    uint8_t slot, bool fix_cursor,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
){
    accept_FRs(
        *PokemonAutomation::global_connection,
        slot, fix_cursor,
        game_to_home_delay_safe,
        auto_fr_duration,
        tolerate_system_update_window_slow
    );
    return 0;
}
void accept_FRs_while_waiting(
    uint8_t slot, uint16_t wait_time,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
){
    accept_FRs_while_waiting(
        *PokemonAutomation::global_connection,
        slot, wait_time,
        game_to_home_delay_safe,
        auto_fr_duration,
        tolerate_system_update_window_slow
    );
}
#endif



namespace PokemonAutomation{




void connect_to_internet(
    const BotBaseContext& context,
    uint16_t open_ycomm_delay,
    uint16_t connect_to_internet_delay
){
    pabb_connect_to_internet params;
    params.open_ycomm_delay = open_ycomm_delay;
    params.connect_to_internet_delay = connect_to_internet_delay;
    context->issue_request<PABB_MSG_COMMAND_CONNECT_TO_INTERNET>(&context.cancelled_bool(), params);
}
void home_to_add_friends(
    const BotBaseContext& context,
    uint8_t user_slot,
    uint8_t scroll_down,
    bool fix_cursor
){
    pabb_home_to_add_friends params;
    params.user_slot = user_slot;
    params.scroll_down = scroll_down;
    params.fix_cursor = fix_cursor;
    context->issue_request<PABB_MSG_COMMAND_HOME_TO_ADD_FRIENDS>(&context.cancelled_bool(), params);
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
        ENTER_SWITCH_POKEMON, EXIT_SWITCH_POKEMON
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


