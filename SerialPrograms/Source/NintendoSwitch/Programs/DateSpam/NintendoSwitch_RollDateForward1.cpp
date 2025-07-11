/*  Nintendo Switch Roll Date Forward 1
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_RollDateForward1.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void roll_date_forward_1_Switch1_wired(ProControllerContext& context, bool fast){
    Milliseconds delay = 24ms;

    ssf_press_button(context, BUTTON_A, 160ms, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 0ms, 2*delay, delay);
    ssf_press_button(context, BUTTON_A, delay);
//    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay);
    ssf_issue_scroll(context, SSF_SCROLL_UP, delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay);
    ssf_press_button(context, BUTTON_A, 0ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0ms, 2*delay, delay);
    ssf_press_button(context, BUTTON_A, 160ms, 80ms);
}
void roll_date_forward_1_Switch1_wireless(ProControllerContext& context){
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_press_button(context, BUTTON_A, 160ms, 3*unit);
    ssf_issue_scroll(context, SSF_SCROLL_UP, unit);

    //  Left scroll in case we missed the date menu and landed in the
    //  language change.
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms, 2*unit, unit);

    ssf_press_button(context, BUTTON_A, unit);
//    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_UP, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_press_button(context, BUTTON_A, 0ms, 2*unit, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0ms, 2*unit, unit);
    ssf_press_button(context, BUTTON_A, 160ms, 3*unit);
}
void roll_date_forward_1_Switch1_sbb(ProControllerContext& context){
    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);

    //  Left scroll in case we missed the date menu and landed in the
    //  language change.
    ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT);

    ssf_press_button_ptv(context, BUTTON_A);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_press_button_ptv(context, BUTTON_A);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
}
void roll_date_forward_1_Switch2_wired(ProControllerContext& context){
    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);
}
void roll_date_forward_1_Switch2_wireless(ProControllerContext& context){
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_UP, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);
}
void roll_date_forward_1(
    ConsoleHandle& console, ProControllerContext& context,
    bool fast
){
    ConsoleType type = console.state().console_type();

    if (is_switch1(type)){
        switch (context->performance_class()){
        case ControllerPerformanceClass::SerialPABotBase_Wired:
            roll_date_forward_1_Switch1_wired(context, fast);
            return;
        case ControllerPerformanceClass::SerialPABotBase_Wireless:
            roll_date_forward_1_Switch1_wireless(context);
            return;
        case ControllerPerformanceClass::SysbotBase:
            roll_date_forward_1_Switch1_sbb(context);
            return;
        default:
            throw InternalProgramError(
                &console.logger(), PA_CURRENT_FUNCTION,
                "Unsupported ControllerPerformanceClass: " + std::to_string((int)context->performance_class())
            );
        }
    }

    if (is_switch2(type)){
        switch (context->performance_class()){
        case ControllerPerformanceClass::SerialPABotBase_Wired:
            roll_date_forward_1_Switch2_wired(context);
            return;
        case ControllerPerformanceClass::SerialPABotBase_Wireless:
            roll_date_forward_1_Switch2_wireless(context);
            return;
        default:
            throw InternalProgramError(
                &console.logger(), PA_CURRENT_FUNCTION,
                "Unsupported ControllerPerformanceClass: " + std::to_string((int)context->performance_class())
            );
        }
        return;
    }

    throw UserSetupError(
        console.logger(),
        "Please select a valid Switch console type."
    );
}











}
}
