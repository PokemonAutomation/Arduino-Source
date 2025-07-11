/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
//#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
//#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateForward1.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateBackwardN.h"
#include "PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_Commands_DateSpam.h"
//#include "PokemonSwSh_Messages_DateSpam.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void rollback_year_skip_forward(
    ConsoleHandle& console, ProControllerContext& context
){
    ConsoleType type = console.state().console_type();
    if (is_switch1(type)){
        roll_date_backward_N(console, context, MAX_YEAR, true);
        ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT, 24ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT, 24ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT, 24ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP, 16ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT, 24ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP, 16ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT, 24ms);
//        ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT, 24ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP, 16ms);
        return;
    }

    if (is_switch2(type)){
        roll_date_backward_N(console, context, MAX_YEAR, true);
        roll_date_forward_1(console, context, true);
        return;
    }

    throw UserSetupError(
        console.logger(),
        "Please select a valid Switch console type."
    );
}


void home_roll_date_enter_game(
    ConsoleHandle& console, ProControllerContext& context,
    bool rollback_year
){
    //  From the Switch home menu, roll the date, then re-enter the game.
    home_to_date_time(console, context, true);

    if (rollback_year){
        rollback_year_skip_forward(console, context);
    }else{
        roll_date_forward_1(console, context, true);
    }

    settings_to_enter_game(context, true);
    resume_game_from_home(console, context, true);
}
void home_roll_date_enter_game_autorollback(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t& year
){
    //  This version automatically handles the 2060 roll-back.
    if (year >= MAX_YEAR){
        home_roll_date_enter_game(console, context, true);
        year = 0;
    }else{
        home_roll_date_enter_game(console, context, false);
    }
    year++;
}





void touch_date_from_home_switch1(
    ConsoleHandle& console, ProControllerContext& context,
    Milliseconds settings_to_home_delay
){
    home_to_date_time(console, context, true);
    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);

    ssf_press_button_ptv(context, BUTTON_A, 0ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_press_button_ptv(context, BUTTON_A, 0ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);

    ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_UP, 16ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT, 0ms);
    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);

    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN, 16ms);
    ssf_press_button(context, BUTTON_HOME, settings_to_home_delay, 80ms);
}
void touch_date_from_home_switch2(
    ConsoleHandle& console, ProControllerContext& context,
    Milliseconds settings_to_home_delay
){
    home_to_date_time(console, context, true);

    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);

    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);

    ssf_press_button(context, BUTTON_HOME, settings_to_home_delay, 80ms);
}
void touch_date_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    Milliseconds settings_to_home_delay
){
    ensure_at_home(console, context);

    ConsoleType type = console.state().console_type();

    if (is_switch1(type)){
        touch_date_from_home_switch1(console, context, settings_to_home_delay);
        return;
    }
    if (is_switch2(type)){
        touch_date_from_home_switch2(console, context, settings_to_home_delay);
        return;
    }

    throw UserSetupError(
        console.logger(),
        "Please select a valid Switch console type."
    );
}




void rollback_hours_from_home_switch1(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t hours,
    Milliseconds settings_to_home_delay
){
    home_to_date_time(console, context, true);
    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);

    ssf_press_button_ptv(context, BUTTON_A, 0ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    for (uint8_t c = 0; c < hours; c++){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
    }
    ssf_press_button_ptv(context, BUTTON_A, 0ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);

    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
    ssf_press_button(context, BUTTON_HOME, settings_to_home_delay, 80ms);
}
void rollback_hours_from_home_switch2(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t hours,
    Milliseconds settings_to_home_delay
){
    home_to_date_time(console, context, true);

    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    for (uint8_t c = 0; c < hours; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 112ms + tv, 2*unit, unit);
    }
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);

    ssf_press_button(context, BUTTON_HOME, settings_to_home_delay, 80ms);
}


void rollback_hours_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t hours,
    Milliseconds settings_to_home_delay
){
    ensure_at_home(console, context);

    ConsoleType type = console.state().console_type();

    if (is_switch1(type)){
        rollback_hours_from_home_switch1(console, context, hours, settings_to_home_delay);
        return;
    }
    if (is_switch2(type)){
        rollback_hours_from_home_switch2(console, context, hours, settings_to_home_delay);
        return;
    }

    throw UserSetupError(
        console.logger(),
        "Please select a valid Switch console type."
    );
}





}

}
}

