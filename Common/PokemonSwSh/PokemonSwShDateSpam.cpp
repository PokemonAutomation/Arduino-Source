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


#if 0
void home_to_date_time(bool to_date_change, bool fast){
    home_to_date_time(*PokemonAutomation::global_connection, to_date_change, fast);
}
void roll_date_forward_1(bool fast){
    roll_date_forward_1(*PokemonAutomation::global_connection, fast);
}
void roll_date_backward_N(uint8_t skips, bool fast){
    roll_date_backward_N(*PokemonAutomation::global_connection, skips, fast);
}
void home_roll_date_enter_game_autorollback(uint8_t* year){
    home_roll_date_enter_game_autorollback(*PokemonAutomation::global_connection, year);
}
void home_roll_date_enter_game(bool rollback_year){
    home_roll_date_enter_game(*PokemonAutomation::global_connection, rollback_year);
}
void touch_date_from_home(uint16_t settings_to_home_delay){
    touch_date_from_home(*PokemonAutomation::global_connection, settings_to_home_delay);
}
void rollback_hours_from_home(uint8_t hours, uint16_t settings_to_home_delay){
    rollback_hours_from_home(*PokemonAutomation::global_connection, hours, settings_to_home_delay);
}
#endif



namespace PokemonAutomation{



void home_to_date_time(const BotBaseContext& context, bool to_date_change, bool fast){
    pabb_home_to_date_time params;
    params.to_date_change = to_date_change;
    params.fast = fast;
    context->issue_request<PABB_MSG_COMMAND_HOME_TO_DATE_TIME>(&context.cancelled_bool(), params);
}
void neutral_date_skip(const BotBaseContext& context){
    pabb_neutral_date_skip params;
    context->issue_request<PABB_MSG_COMMAND_NEUTRAL_DATE_SKIP>(&context.cancelled_bool(), params);
}
void roll_date_forward_1(const BotBaseContext& context, bool fast){
    pabb_roll_date_forward_1 params;
    params.fast = fast;
    context->issue_request<PABB_MSG_COMMAND_ROLL_DATE_FORWARD_1>(&context.cancelled_bool(), params);
}
void roll_date_backward_N(const BotBaseContext& context, uint8_t skips, bool fast){
    pabb_roll_date_backward_N params;
    params.skips = skips;
    params.fast = fast;
    context->issue_request<PABB_MSG_COMMAND_ROLL_DATE_BACKWARD_N>(&context.cancelled_bool(), params);
}
void home_roll_date_enter_game_autorollback(const BotBaseContext& context, uint8_t* year){
    //  This version automatically handles the 2060 roll-back.
    if (*year >= MAX_YEAR){
        home_roll_date_enter_game(context, true);
        *year = 0;
    }else{
        home_roll_date_enter_game(context, false);
    }
    (*year)++;
}
void home_roll_date_enter_game(const BotBaseContext& context, bool rollback_year){
    pabb_home_roll_date_enter_game params;
    params.rollback_year = rollback_year;
    context->issue_request<PABB_MSG_COMMAND_HOME_ROLL_DATE_ENTER_GAME>(&context.cancelled_bool(), params);
}
void touch_date_from_home(const BotBaseContext& context, uint16_t settings_to_home_delay){
    pabb_touch_date_from_home params;
    params.settings_to_home_delay = settings_to_home_delay;
    context->issue_request<PABB_MSG_COMMAND_TOUCH_DATE_FROM_HOME>(&context.cancelled_bool(), params);
}
void rollback_hours_from_home(const BotBaseContext& context, uint8_t hours, uint16_t settings_to_home_delay){
    pabb_rollback_hours_from_home params;
    params.hours = hours;
    params.settings_to_home_delay = settings_to_home_delay;
    context->issue_request<PABB_MSG_COMMAND_ROLLBACK_HOURS_FROM_HOME>(&context.cancelled_bool(), params);
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
            ss << ", to_date_change = " << params->to_date_change;
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
            ss << ", hours = " << params->hours;
            ss << ", settings_to_home_delay = " << params->settings_to_home_delay;
            return ss.str();
        }
    );
    return 0;
}

int init_PokemonSwShDateSpam = register_message_converters_pokemon_date_spam();


}

