/*  Nintendo Switch Roll Date Backward N
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_RollDateBackwardN.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void roll_date_backward_N_Switch1_wired(ProControllerContext& context, uint8_t skips, bool fast){
    uint8_t scroll_delay = fast ? 3 : 4;
    uint8_t up_delay = 3;
    ssf_press_button(context, BUTTON_A, 20, 10);
    for (uint8_t c = 0; c < skips - 1; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, up_delay);
    }
#if 0
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);
//        ssf_press_button(context, BUTTON_A, up_delay);
#else
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, up_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
#endif
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    for (uint8_t c = 0; c < skips - 1; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, up_delay);
    }
#if 0
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);
    ssf_press_button(context, BUTTON_A, up_delay);
#else
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, up_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
#endif
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);

#if 0
//        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0);
#else
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, scroll_delay);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, scroll_delay);
    ssf_press_button(context, BUTTON_A);
#endif

    ssf_press_button(context, BUTTON_A, 20, 10);
}
void roll_date_backward_N_Switch1_wireless(ProControllerContext& context, uint8_t skips){
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_press_button(context, BUTTON_A, 160ms, 3*unit);

    for (uint8_t c = 0; c < skips - 1; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    }

#if 0
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0ms, 2*unit, unit);
    ssf_press_button(context, BUTTON_A, unit);
#else
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
#endif
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    for (uint8_t c = 0; c < skips - 1; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    }
#if 0
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0ms, 2*unit, unit);
    ssf_press_button(context, BUTTON_A, unit);
#else
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
#endif
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0ms, 2*unit, unit);
    ssf_press_button(context, BUTTON_A, 160ms, 3*unit);
}
void roll_date_backward_N_Switch1_sbb(ProControllerContext& context, uint8_t skips){
    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
    if (skips >= 60){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN, 6000ms, 6000ms);
    }else{
        for (uint8_t c = 0; c < skips; c++){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
        }
    }

    //  Left scroll in case we missed the date menu and landed in the
    //  language change.
    ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT);

    ssf_press_button_ptv(context, BUTTON_A);
//    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    if (skips >= 60){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN, 6000ms, 6000ms);
    }else{
        for (uint8_t c = 0; c < skips; c++){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
        }
    }
    ssf_press_button_ptv(context, BUTTON_A);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
}
void roll_date_backward_N_Switch2_wired(ProControllerContext& context, uint8_t skips){
    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    if (skips >= 60){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 4160ms, 4160ms, 24ms);
    }else{
        for (uint8_t c = 0; c < skips; c++){
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 112ms, 48ms, 24ms);
        }
    }
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    if (skips >= 60){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 4160ms, 4160ms, 24ms);
    }else{
        for (uint8_t c = 0; c < skips; c++){
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 112ms, 48ms, 24ms);
        }
    }
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);
}
void roll_date_backward_N(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t skips, bool fast
){
    ConsoleType type = console.state().console_type();

    if (is_switch1(type)){
        switch (context->performance_class()){
        case ControllerPerformanceClass::SerialPABotBase_Wired_125Hz:
            roll_date_backward_N_Switch1_wired(context, skips, fast);
            return;
        case ControllerPerformanceClass::SerialPABotBase_Wireless_ESP32:
            roll_date_backward_N_Switch1_wireless(context, skips);
            return;
        case ControllerPerformanceClass::SysbotBase:
            roll_date_backward_N_Switch1_sbb(context, skips);
            return;
        default:
            throw InternalProgramError(
                &console.logger(), PA_CURRENT_FUNCTION,
                "Unsupported ControllerPerformanceClass: " + std::to_string((int)context->performance_class())
            );
        }
    }

    if (is_switch2(type)){
        roll_date_backward_N_Switch2_wired(context, skips);
        return;
    }

    throw UserSetupError(
        console.logger(),
        "Please select a valid Switch console type."
    );
}












}
}










