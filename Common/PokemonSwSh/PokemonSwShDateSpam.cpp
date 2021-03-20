/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwShDateSpam.h"

using namespace PokemonAutomation;

void home_to_date_time(bool fast){
    home_to_date_time(*global_connection, fast);
}
void home_to_date_time(BotBase& device, bool fast){
    pabb_home_to_date_time params;
    params.fast = fast;
    device.issue_request<PABB_MSG_COMMAND_HOME_TO_DATE_TIME>(params);
}

void roll_date_forward_1(bool fast){
    roll_date_forward_1(*global_connection, fast);
}
void roll_date_forward_1(BotBase& device, bool fast){
    pabb_roll_date_forward_1 params;
    params.fast = fast;
    device.issue_request<PABB_MSG_COMMAND_ROLL_DATE_FORWARD_1>(params);
}

void roll_date_backward_N(uint8_t skips, bool fast){
    roll_date_backward_N(*global_connection, skips, fast);
}
void roll_date_backward_N(BotBase& device, uint8_t skips, bool fast){
    pabb_roll_date_backward_N params;
    params.skips = skips;
    params.fast = fast;
    device.issue_request<PABB_MSG_COMMAND_ROLL_DATE_BACKWARD_N>(params);
}

void home_roll_date_enter_game_autorollback(uint8_t* year){
    home_roll_date_enter_game_autorollback(*global_connection, year);
}
void home_roll_date_enter_game_autorollback(BotBase& device, uint8_t* year){
    //  This version automatically handles the 2060 roll-back.
    if (*year >= MAX_YEAR){
        home_roll_date_enter_game(device, true);
        *year = 0;
    }else{
        home_roll_date_enter_game(device, false);
    }
    (*year)++;
}

void home_roll_date_enter_game(bool rollback_year){
    home_roll_date_enter_game(*global_connection, rollback_year);
}
void home_roll_date_enter_game(BotBase& device, bool rollback_year){
    pabb_home_roll_date_enter_game params;
    params.rollback_year = rollback_year;
    device.issue_request<PABB_MSG_COMMAND_HOME_ROLL_DATE_ENTER_GAME>(params);
}

void touch_date_from_home(uint16_t settings_to_home_delay){
    touch_date_from_home(*global_connection, settings_to_home_delay);
}
void touch_date_from_home(BotBase& device, uint16_t settings_to_home_delay){
    pabb_touch_date_from_home params;
    params.settings_to_home_delay = settings_to_home_delay;
    device.issue_request<PABB_MSG_COMMAND_TOUCH_DATE_FROM_HOME>(params);
}

void rollback_hours_from_home(uint8_t hours, uint16_t settings_to_home_delay){
    rollback_hours_from_home(*global_connection, hours, settings_to_home_delay);
}
void rollback_hours_from_home(BotBase& device, uint8_t hours, uint16_t settings_to_home_delay){
    pabb_rollback_hours_from_home params;
    params.hours = hours;
    params.settings_to_home_delay = settings_to_home_delay;
    device.issue_request<PABB_MSG_COMMAND_ROLLBACK_HOURS_FROM_HOME>(params);
}



int register_message_converters_pokemon_date_spam(){
    register_message_converter(
        PABB_MSG_COMMAND_HOME_TO_DATE_TIME,
        [](const std::string& body){
            std::stringstream ss;
            ss << "home_to_date_time() - ";
            if (body.size() != sizeof(pabb_home_to_date_time)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_home_to_date_time*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", fast = " << params->fast;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_ROLL_DATE_FORWARD_1,
        [](const std::string& body){
            std::stringstream ss;
            ss << "roll_date_forward_1() - ";
            if (body.size() != sizeof(pabb_roll_date_forward_1)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_roll_date_forward_1*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", fast = " << params->fast;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_ROLL_DATE_BACKWARD_N,
        [](const std::string& body){
            std::stringstream ss;
            ss << "roll_date_backward_N() - ";
            if (body.size() != sizeof(pabb_roll_date_backward_N)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_roll_date_backward_N*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", skips = " << (unsigned)params->skips;
            ss << ", fast = " << params->fast;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_HOME_ROLL_DATE_ENTER_GAME,
        [](const std::string& body){
            std::stringstream ss;
            ss << "home_roll_date_enter_game() - ";
            if (body.size() != sizeof(pabb_home_roll_date_enter_game)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_home_roll_date_enter_game*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", rollback_year = " << params->rollback_year;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_TOUCH_DATE_FROM_HOME,
        [](const std::string& body){
            std::stringstream ss;
            ss << "home_roll_date_enter_game_autorollback() - ";
            if (body.size() != sizeof(pabb_touch_date_from_home)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_touch_date_from_home*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_ROLLBACK_HOURS_FROM_HOME,
        [](const std::string& body){
            std::stringstream ss;
            ss << "rollback_hours_from_home() - ";
            if (body.size() != sizeof(pabb_rollback_hours_from_home)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_rollback_hours_from_home*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", rollback_year = " << params->settings_to_home_delay;
            return ss.str();
        }
    );
    return 0;
}

int init_PokemonSwShDateSpam = register_message_converters_pokemon_date_spam();

