/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwShAutoHosts.h"

using namespace PokemonAutomation;

void connect_to_internet(uint16_t open_ycomm_delay, uint16_t connect_to_internet_delay){
    connect_to_internet(*global_connection, open_ycomm_delay, connect_to_internet_delay);
}
void connect_to_internet(
    BotBase& device,
    uint16_t open_ycomm_delay,
    uint16_t connect_to_internet_delay
){
    pabb_connect_to_internet params;
    params.open_ycomm_delay = open_ycomm_delay;
    params.connect_to_internet_delay = connect_to_internet_delay;
    device.issue_request<PABB_MSG_COMMAND_CONNECT_TO_INTERNET>(params);
}
void home_to_add_friends(uint8_t user_slot, uint8_t scroll_down, bool fix_cursor){
    home_to_add_friends(*global_connection, user_slot, scroll_down, fix_cursor);
}
void home_to_add_friends(BotBase& device, uint8_t user_slot, uint8_t scroll_down, bool fix_cursor){
    pabb_home_to_add_friends params;
    params.user_slot = user_slot;
    params.scroll_down = scroll_down;
    params.fix_cursor = fix_cursor;
    device.issue_request<PABB_MSG_COMMAND_HOME_TO_ADD_FRIENDS>(params);
}
uint16_t accept_FRs(
    uint8_t slot, bool fix_cursor,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
){
    accept_FRs(
        *global_connection,
        slot, fix_cursor,
        game_to_home_delay_safe,
        auto_fr_duration,
        tolerate_system_update_window_slow
    );
    return 0;
}
uint16_t accept_FRs(
    BotBase& device,
    uint8_t slot, bool fix_cursor,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
){
    pabb_accept_FRs params;
    params.slot = slot;
    params.fix_cursor = fix_cursor;
    params.game_to_home_delay_safe = game_to_home_delay_safe;
    params.auto_fr_duration = auto_fr_duration;
    params.tolerate_system_update_window_slow = tolerate_system_update_window_slow;
    device.issue_request<PABB_MSG_COMMAND_ACCEPT_FRS>(params);
    return 0;
}
void accept_FRs_while_waiting(
    uint8_t slot, uint16_t wait_time,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
){
    accept_FRs_while_waiting(
        *global_connection,
        slot, wait_time,
        game_to_home_delay_safe,
        auto_fr_duration,
        tolerate_system_update_window_slow
    );
}
void accept_FRs_while_waiting(
    BotBase& device,
    uint8_t slot, uint16_t wait_time,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
){
    pabb_accept_FRs_while_waiting params;
    params.slot = slot;
    params.wait_time = wait_time;
    params.game_to_home_delay_safe = game_to_home_delay_safe;
    params.auto_fr_duration = auto_fr_duration;
    params.tolerate_system_update_window_slow = tolerate_system_update_window_slow;
    device.issue_request<PABB_MSG_COMMAND_ACCEPT_FRS_WHILE_WAITING>(params);
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
    register_message_converter(
        PABB_MSG_COMMAND_ACCEPT_FRS,
        [](const std::string& body){
            std::stringstream ss;
            ss << "accept_FRs() - ";
            if (body.size() != sizeof(pabb_accept_FRs)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_accept_FRs*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", slot = " << (unsigned)params->slot;
            ss << ", fix_cursor = " << params->fix_cursor;
            ss << ", game_to_home_delay_safe = " << params->game_to_home_delay_safe;
            ss << ", auto_fr_duration = " << params->auto_fr_duration;
            ss << ", tolerate_system_update_window_slow = " << params->tolerate_system_update_window_slow;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_ACCEPT_FRS_WHILE_WAITING,
        [](const std::string& body){
            std::stringstream ss;
            ss << "accept_FRs_while_waiting() - ";
            if (body.size() != sizeof(pabb_accept_FRs_while_waiting)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_accept_FRs_while_waiting*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", slot = " << (unsigned)params->slot;
            ss << ", wait_time = " << params->wait_time;
            ss << ", game_to_home_delay_safe = " << params->game_to_home_delay_safe;
            ss << ", auto_fr_duration = " << params->auto_fr_duration;
            ss << ", tolerate_system_update_window_slow = " << params->tolerate_system_update_window_slow;
            return ss.str();
        }
    );
    return 0;
}
int init_PokemonSwShAutoHosts = register_message_converters_pokemon_autohosting();

