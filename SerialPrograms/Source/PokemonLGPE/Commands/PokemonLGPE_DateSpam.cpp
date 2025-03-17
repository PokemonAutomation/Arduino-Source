/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/NintendoSwitch_Navigation.h"
#include "PokemonLGPE_DateSpam.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

void roll_date_forward_1(JoyconContext& context){
    /*
    uint8_t scroll_delay = fast ? 3 : 4;
    uint8_t up_delay = fast ? 2 : 3;

    pbf_press_button(context, BUTTON_A, 20, 10);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 0);
    pbf_press_button(context, BUTTON_A, up_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_UP, up_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    pbf_press_button(context, BUTTON_A, 0);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0);
    pbf_press_button(context, BUTTON_A, 20, 10);
    */
}
void roll_date_backward_N(JoyconContext& context, uint8_t skips){
    /*
    //  If (fast == true) this will run faster, but slightly less reliably.

    if (skips == 0){
        return;
    }

    Milliseconds tv = context->timing_variation();
    uint8_t scroll_delay = fast ? 3 : 4;
    uint8_t up_delay = 3;

    pbf_press_button(context, BUTTON_A, 20, 10);
    for (uint8_t c = 0; c < skips - 1; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, up_delay);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);
    pbf_press_button(context, BUTTON_A, up_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    for (uint8_t c = 0; c < skips - 1; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, up_delay);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);
    pbf_press_button(context, BUTTON_A, up_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0);
    pbf_press_button(context, BUTTON_A, 20, 10);
    */
}
void rollback_hours_from_home(
    JoyconContext& context,
    uint8_t hours,
    Milliseconds settings_to_home_delay
){
    /*
    home_to_date_time(context, true, false);
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

    pbf_press_button(context, BUTTON_A, 160ms, 80ms);
    ssf_press_button(context, BUTTON_HOME, settings_to_home_delay, 80ms);
    */
}





}

}
}

