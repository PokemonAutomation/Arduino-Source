/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh_Messages_DateSpam.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void home_to_date_time(BotBaseContext& context, bool to_date_change, bool fast){
    context.issue_request(
        DeviceRequest_home_to_date_time(to_date_change, fast)
    );
}
void neutral_date_skip(BotBaseContext& context){
    context.issue_request(
        DeviceRequest_neutral_date_skip()
    );
}
void roll_date_forward_1(BotBaseContext& context, bool fast){
    context.issue_request(
        DeviceRequest_roll_date_forward_1(fast)
    );
}
void roll_date_backward_N(BotBaseContext& context, uint8_t skips, bool fast){
    context.issue_request(
        DeviceRequest_roll_date_backward_N(skips, fast)
    );
}
void home_roll_date_enter_game_autorollback(BotBaseContext& context, uint8_t* year){
    //  This version automatically handles the 2060 roll-back.
    if (*year >= MAX_YEAR){
        home_roll_date_enter_game(context, true);
        *year = 0;
    }else{
        home_roll_date_enter_game(context, false);
    }
    (*year)++;
}
void home_roll_date_enter_game(BotBaseContext& context, bool rollback_year){
    context.issue_request(
        DeviceRequest_home_roll_date_enter_game(rollback_year)
    );
}
void touch_date_from_home(BotBaseContext& context, uint16_t settings_to_home_delay){
    context.issue_request(
        DeviceRequest_touch_date_from_home(settings_to_home_delay)
    );
}
void rollback_hours_from_home(BotBaseContext& context, uint8_t hours, uint16_t settings_to_home_delay){
    context.issue_request(
        DeviceRequest_rollback_hours_from_home(hours, settings_to_home_delay)
    );
}





int register_message_converters_pokemon_date_spam(){
    register_message_converter(
        PABB_MSG_COMMAND_HOME_TO_DATE_TIME,
        [](const std::string& body){
            std::ostringstream ss;
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
        PABB_MSG_COMMAND_NEUTRAL_DATE_SKIP,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "neutral_date_skip() - ";
            if (body.size() != sizeof(pabb_neutral_date_skip)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_neutral_date_skip*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_ROLL_DATE_FORWARD_1,
        [](const std::string& body){
            std::ostringstream ss;
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
            std::ostringstream ss;
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
            std::ostringstream ss;
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
            std::ostringstream ss;
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
            std::ostringstream ss;
            ss << "rollback_hours_from_home() - ";
            if (body.size() != sizeof(pabb_rollback_hours_from_home)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_rollback_hours_from_home*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", hours = " << (int)params->hours;
            ss << ", settings_to_home_delay = " << params->settings_to_home_delay;
            return ss.str();
        }
    );
    return 0;
}

int init_PokemonSwShDateSpam = register_message_converters_pokemon_date_spam();


}
}

