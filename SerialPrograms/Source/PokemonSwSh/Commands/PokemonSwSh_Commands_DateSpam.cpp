/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_Commands_DateSpam.h"
//#include "PokemonSwSh_Messages_DateSpam.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void home_to_date_time(SwitchControllerContext& context, bool to_date_change, bool fast){
#if 0
    context.issue_request(
        DeviceRequest_home_to_date_time(to_date_change, fast)
    );
#else
    //  If (fast == true) this will run faster, but slightly less accurately.

    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 4);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 4);

    //  Down twice in case we drop one.
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 4);

    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0);

    //  Two A presses in case we drop the 1st one.
    ssf_press_button(context, BUTTON_A, 3);
    ssf_press_button(context, BUTTON_A, 3);

    //  Just button mash it. lol
    uint8_t stop0 = fast ? 51 : 52;
    for (uint8_t c = 0; c < stop0; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
    }
//    ssf_issue_scroll1(SSF_SCROLL_RIGHT, fast ? 40 : 45);
//    stop0 = fast ? 14 : 15;
    for (uint8_t c = 0; c < 15; c++){
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    }

    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 10);
    ssf_press_dpad(context, DPAD_DOWN, 45, 40);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);

    if (!to_date_change){
        ssf_press_button(context, BUTTON_A, 45);
        return;
    }

    ssf_press_button(context, BUTTON_A, 3);
    uint8_t stop1 = fast ? 10 : 12;
    for (uint8_t c = 0; c < stop1; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);


//    //  Insert this to move the cursor away from sleep if we messed up.
//    ssf_issue_scroll1(SSF_SCROLL_LEFT, 0);
#endif
}


namespace PokemonSwSh{

void neutral_date_skip(SwitchControllerContext& context){
#if 0
    context.issue_request(
        DeviceRequest_neutral_date_skip()
    );
#else
    ssf_press_button(context, BUTTON_A, 20, 10);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 0);
    ssf_press_button(context, BUTTON_A, 2);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
//    ssf_press_button(context, BUTTON_A, 2);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0);
    ssf_press_button(context, BUTTON_A, 20, 10);
    ssf_press_button(context, BUTTON_A, 20, 10);
    for (uint8_t c = 0; c < 6; c++){
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 3);
    }
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);
#endif
}
void roll_date_forward_1(SwitchControllerContext& context, bool fast){
#if 0
    context.issue_request(
        DeviceRequest_roll_date_forward_1(fast)
    );
#else
    //  If (fast == true) this will run faster, but slightly less reliably.
    uint8_t scroll_delay = fast ? 3 : 4;
    uint8_t up_delay = fast ? 2 : 3;

    ssf_press_button(context, BUTTON_A, 20, 10);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 0);
    ssf_press_button(context, BUTTON_A, up_delay);
//    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_UP, up_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_press_button(context, BUTTON_A, 0);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0);
    ssf_press_button(context, BUTTON_A, 20, 10);
#endif
}
void roll_date_backward_N(SwitchControllerContext& context, uint8_t skips, bool fast){
#if 0
    context.issue_request(
        DeviceRequest_roll_date_backward_N(skips, fast)
    );
#else
    if (skips == 0){
        return;
    }

    //  If (fast == true) this will run faster, but slightly less reliably.
    uint8_t scroll_delay = fast ? 3 : 4;
    uint8_t up_delay = 3;

    ssf_press_button(context, BUTTON_A, 20, 10);
    for (uint8_t c = 0; c < skips - 1; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, up_delay);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);
    ssf_press_button(context, BUTTON_A, up_delay);
//    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    for (uint8_t c = 0; c < skips - 1; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, up_delay);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);
    ssf_press_button(context, BUTTON_A, up_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0);
//    if (!fast){
//        //  Add an extra one in case one is dropped.
//        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay);
//    }
    ssf_press_button(context, BUTTON_A, 20, 10);
#endif
}
void home_roll_date_enter_game_autorollback(
    VideoStream& stream, SwitchControllerContext& context,
    uint8_t& year
){
    //  This version automatically handles the 2060 roll-back.
    if (year >= MAX_YEAR){
        home_roll_date_enter_game(stream, context, true);
        year = 0;
    }else{
        home_roll_date_enter_game(stream, context, false);
    }
    year++;
}
void home_roll_date_enter_game(
    VideoStream& stream, SwitchControllerContext& context,
    bool rollback_year
){
#if 0
    context.issue_request(
        DeviceRequest_home_roll_date_enter_game(rollback_year)
    );
#else
    //  From the Switch home menu, roll the date, then re-enter the game.
    home_to_date_time(context, true, true);

    if (rollback_year){
        roll_date_backward_N(context, MAX_YEAR, true);
        ssf_press_button(context, BUTTON_A, 20, 10);
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 3);
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 3);
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 3);
        ssf_issue_scroll(context, SSF_SCROLL_UP, 2);
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 3);
        ssf_issue_scroll(context, SSF_SCROLL_UP, 2);
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 3);
//        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 3);
        ssf_issue_scroll(context, SSF_SCROLL_UP, 2);
    }else{
        roll_date_forward_1(context, true);
    }

    settings_to_enter_game(context, true);
#endif

    resume_game_from_home(stream, context, true);
}
void touch_date_from_home(SwitchControllerContext& context, uint16_t settings_to_home_delay){
#if 0
    context.issue_request(
        DeviceRequest_touch_date_from_home(settings_to_home_delay)
    );
#else
    //  Touch the date without changing it. This prevents unintentional rollovers.

    home_to_date_time(context, true, true);
    ssf_press_button(context, BUTTON_A, 20, 10);

    ssf_press_button(context, BUTTON_A, 0);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_press_button(context, BUTTON_A, 0);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);

    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 2);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0);
    ssf_press_button(context, BUTTON_A, 20, 10);

    ssf_press_button(context, BUTTON_A, 20, 10);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 2);
    ssf_press_button(context, BUTTON_HOME, settings_to_home_delay, 10);
#endif
}
void rollback_hours_from_home(SwitchControllerContext& context, uint8_t hours, uint16_t settings_to_home_delay){
#if 0
    context.issue_request(
        DeviceRequest_rollback_hours_from_home(hours, settings_to_home_delay)
    );
#else
    home_to_date_time(context, true, false);
    ssf_press_button(context, BUTTON_A, 20, 10);

    ssf_press_button(context, BUTTON_A, 0);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    for (uint8_t c = 0; c < hours; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
    }
    ssf_press_button(context, BUTTON_A, 0);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);

    ssf_press_button(context, BUTTON_A, 20, 10);
    ssf_press_button(context, BUTTON_HOME, settings_to_home_delay, 10);
#endif
}




#if 0
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
            ss << "touch_date_from_home() - ";
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
#endif


}

}
}

