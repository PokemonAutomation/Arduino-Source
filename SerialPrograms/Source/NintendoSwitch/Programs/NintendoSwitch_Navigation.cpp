/*  Nintendo Switch Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void home_to_date_time(ProControllerContext& context, bool to_date_change, bool fast){
    //  If (fast == true) this will run faster, but slightly less accurately.

    Milliseconds timing_variation = context->timing_variation();
    if (timing_variation == 0ms){
        //  Fast version for tick-precise.

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
//        ssf_issue_scroll1(SSF_SCROLL_RIGHT, fast ? 40 : 45);
//        stop0 = fast ? 14 : 15;
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

//        //  Insert this to move the cursor away from sleep if we messed up.
//        ssf_issue_scroll1(SSF_SCROLL_LEFT, 0);
    }else{
        //  Slow version for tick-imprecise controllers.

        ssf_do_nothing(context, 1500ms);

        ssf_issue_scroll(context, SSF_SCROLL_RIGHT);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT);

        //  Down twice in case we drop one.
        ssf_issue_scroll(context, SSF_SCROLL_DOWN);
//        ssf_issue_scroll(context, SSF_SCROLL_DOWN);

        ssf_issue_scroll(context, SSF_SCROLL_RIGHT);

        //  Two A presses in case we drop the 1st one.
        ssf_mash1_button(context, BUTTON_A, 200ms);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 2500ms, 2500ms);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 500ms, 500ms);

        ssf_issue_scroll(context, SSF_SCROLL_DOWN);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 500ms, 300ms);   //  Scroll down
        ssf_issue_scroll(context, SSF_SCROLL_DOWN);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN);

        if (!to_date_change){
            ssf_press_button(context, BUTTON_A);
            return;
        }

        ssf_press_button(context, BUTTON_A, 1000ms, timing_variation, timing_variation);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN);
    }
}




}
}
