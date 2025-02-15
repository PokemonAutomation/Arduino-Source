/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/NintendoSwitch_Navigation.h"
#include "PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_Commands_DateSpam.h"
//#include "PokemonSwSh_Messages_DateSpam.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

void neutral_date_skip(ProControllerContext& context){
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
}
void roll_date_forward_1(ProControllerContext& context, bool fast){
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
}
void roll_date_backward_N(ProControllerContext& context, uint8_t skips, bool fast){
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
}
void home_roll_date_enter_game_autorollback(
    VideoStream& stream, ProControllerContext& context,
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
    VideoStream& stream, ProControllerContext& context,
    bool rollback_year
){
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

    resume_game_from_home(stream, context, true);
}
void touch_date_from_home(ProControllerContext& context, Milliseconds settings_to_home_delay){
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
    ssf_press_button(context, BUTTON_HOME, settings_to_home_delay, 80ms);
}
void rollback_hours_from_home(
    ProControllerContext& context,
    uint8_t hours,
    Milliseconds settings_to_home_delay
){
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
    ssf_press_button(context, BUTTON_HOME, settings_to_home_delay, 80ms);
}





}

}
}

