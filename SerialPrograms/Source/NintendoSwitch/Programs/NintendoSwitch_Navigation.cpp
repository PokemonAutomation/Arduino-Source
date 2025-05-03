/*  Nintendo Switch Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void home_to_date_time(ProControllerContext& context, bool to_date_change, bool fast){
    switch (context->performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired_125Hz:{
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 4);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 4);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 4);

        //  Down twice in case we drop one.
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 4);

        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0);

        //  Two A presses in case we drop the 1st one.
        ssf_press_button(context, BUTTON_A, 3);
        ssf_press_button(context, BUTTON_A, 3);

        //  Just button mash it. lol
        {
            auto iterations = Milliseconds(1200) / 24ms + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
            }while (--iterations);
        }

        //  Scroll left and press A to exit the sleep menu if we happened to
        //  land there.
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 3);
        ssf_press_button(context, BUTTON_A, 3);

        {
            auto iterations = Milliseconds(312) / 24ms + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms);
            }while (--iterations);
        }

        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 10);
        ssf_press_dpad(context, DPAD_DOWN, 45, 40);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);

        if (!to_date_change){
            //  Double up this A press in case one is dropped.
            ssf_press_button(context, BUTTON_A, 3);
            ssf_press_button(context, BUTTON_A, 45);
            return;
        }

        //  Double up this A press in case one is dropped.
        ssf_press_button(context, BUTTON_A, 3);
        ssf_press_button(context, BUTTON_A, 3);
        {
            auto iterations = Milliseconds(240) / 24ms + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
            }while (--iterations);
        }
//        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);

        //  Left scroll in case we missed landed in the language change or sleep
        //  confirmation menus.
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms);

        break;
    }
    case ControllerPerformanceClass::SerialPABotBase_Wireless_ESP32:{
        Milliseconds tv = context->timing_variation();
        Milliseconds unit = 24ms + tv;

        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);

        //  Down twice in case we drop one.
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);

        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms, 2*unit, unit);

        //  Press A multiple times to make sure one goes through.
        pbf_press_button(context, BUTTON_A, 2*unit, unit);
        pbf_press_button(context, BUTTON_A, 2*unit, unit);
        pbf_press_button(context, BUTTON_A, 2*unit, unit);

        //  Just button mash it. lol
        {
            auto iterations = Milliseconds(1100) / unit + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
            }while (--iterations);
        }

        //  Scroll left and press A to exit the sleep menu if we happened to
        //  land there.
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, unit);
        ssf_press_button(context, BUTTON_A, unit, 2*unit, unit);

        {
            auto iterations = Milliseconds(312) / unit + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
            }while (--iterations);
        }

        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 400ms, 2*unit, unit);
        ssf_press_dpad(context, DPAD_DOWN, 360ms, 304ms);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);

        if (!to_date_change){
            //  Double up this A press in case one is dropped.
            ssf_press_button(context, BUTTON_A, unit);
            ssf_press_button(context, BUTTON_A, 360ms, 2*unit, unit);
            return;
        }

        //  Double up this A press in case one is dropped.
        ssf_press_button(context, BUTTON_A, unit);
        ssf_press_button(context, BUTTON_A, unit);
        {
            auto iterations = Milliseconds(240) / unit + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
            }while (--iterations);
        }

        //  Left scroll in case we missed landed in the language change or sleep
        //  confirmation menus.
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms, 2*unit, unit);

        break;
    }
    default:{
        //  Slow version for tick-imprecise controllers.

        Milliseconds tv = context->timing_variation();
//        ssf_do_nothing(context, 1500ms);

        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);

        //  Down twice in case we drop one.
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
//        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);

        ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT);

        //  Press A multiple times to make sure one goes through.
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



void home_to_date_time(JoyconContext& context, bool to_date_change){
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 100ms + tv;

    //From ControllerPerformanceClass::SerialPABotBase_Wireless_ESP32
    //as Joycon will only have that controller type

    pbf_move_joystick(context, 255, 128, 2*unit, unit);
    pbf_move_joystick(context, 255, 128, 2*unit, unit);
    pbf_move_joystick(context, 255, 128, 2*unit, unit);

    //  Down twice in case we drop one.
    pbf_move_joystick(context, 128, 255, 2*unit, unit);
    pbf_move_joystick(context, 128, 255, 2*unit, unit);

    pbf_move_joystick(context, 0, 128, 2*unit, unit);

    //  Press A multiple times to make sure one goes through.
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);

    // Scroll to System, move right to top option (update)
    pbf_move_joystick(context, 128, 255, 2500ms, unit);
    pbf_move_joystick(context, 255, 128, 500ms, unit);
    
    // To date/time
    pbf_move_joystick(context, 128, 255, 2*unit, unit);
    pbf_move_joystick(context, 128, 255, 2*unit, unit);
    context.wait_for_all_requests();
    pbf_move_joystick(context, 128, 255, 525ms, unit);
    //pbf_move_joystick(context, 128, 255, 365ms, 305ms);
    pbf_move_joystick(context, 128, 255, 2*unit, unit);
    //pbf_move_joystick(context, 128, 255, 2*unit, unit);
    context.wait_for_all_requests();
    
    if (!to_date_change){
        ssf_press_button(context, BUTTON_A, 360ms, 2*unit, unit);
        return;
    }

    //ssf_press_button(context, BUTTON_A, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    context.wait_for_all_requests();
    {
        auto iterations = Milliseconds(216) / unit + 1;
        do{
            pbf_move_joystick(context, 128, 255, 2*unit, unit);
        }while (--iterations);
    }
    pbf_move_joystick(context, 128, 255, 2*unit, 0ms);
}




}
}
