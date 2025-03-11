/*  Nintendo Switch Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void home_to_date_time(ProControllerContext& context, bool to_date_change, bool fast){
    //  If (fast == true) this will run faster, but slightly less accurately.

    Milliseconds ticksize = context->ticksize();
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = round_up_to_ticksize(ticksize, 17ms);
    if (tv == 0ms){
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
        {
            auto iterations = Milliseconds(1200) / unit + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
            }while (--iterations);
        }
        {
            auto iterations = Milliseconds(336) / unit + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
            }while (--iterations);
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
        {
            auto iterations = Milliseconds(216) / unit + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
            }while (--iterations);
        }
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);

//        //  Insert this to move the cursor away from sleep if we messed up.
//        ssf_issue_scroll1(SSF_SCROLL_LEFT, 0);
    }else{
        //  Slow version for tick-imprecise controllers.

//        ssf_do_nothing(context, 1500ms);

        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);

        //  Down twice in case we drop one.
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
//        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);

        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);

        //  Two A presses in case we drop the 1st one.
        ssf_mash1_button(context, BUTTON_A, 200ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN, 2500ms, 2500ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT, 500ms, 500ms);

        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 500ms, tv, tv);
        ssf_press_right_joystick(context, 128, 224, 1000ms, 300ms, tv);
//        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 1000ms, 250ms, tv);  //  Scroll down
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);

        if (!to_date_change){
            ssf_press_button_ptv(context, BUTTON_A);
            return;
        }

        ssf_press_button_ptv(context, BUTTON_A, 1000ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
    }
}




}
}
