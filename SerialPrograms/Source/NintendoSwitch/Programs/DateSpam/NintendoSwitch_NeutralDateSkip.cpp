/*  Nintendo Switch Neutral Date Skip
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void neutral_date_skip_switch1_wired(ProControllerContext& context){
    ssf_press_button(context, BUTTON_A, 160ms, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 0ms, 40ms, 24ms);
    ssf_press_button(context, BUTTON_A, 16ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
//    ssf_press_button(context, BUTTON_A, 2);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 160ms, 80ms);
    ssf_press_button(context, BUTTON_A, 160ms, 80ms);
    for (uint8_t c = 0; c < 6; c++){
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    }
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0ms, 48ms, 24ms);
}
void neutral_date_skip_switch1_wireless(ProControllerContext& context){
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_press_button(context, BUTTON_A, 160ms, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 0ms, 2*unit, unit);
    ssf_press_button(context, BUTTON_A, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
//    ssf_press_button(context, BUTTON_A, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_press_button(context, BUTTON_A, 160ms, 80ms);
    ssf_press_button(context, BUTTON_A, 160ms, 80ms);
    for (uint8_t c = 0; c < 6; c++){
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, unit);
    }
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0ms, 2*unit, unit);
}
void neutral_date_skip_switch1_sbb(ProControllerContext& context){
    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
    ssf_press_button_ptv(context, BUTTON_A);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
    ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
    for (uint8_t c = 0; c < 6; c++){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT);
    }
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
}
void neutral_date_skip_switch2_wired(ProControllerContext& context){
    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);
    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);
}
void neutral_date_skip_switch2_wireless(ProControllerContext& context){
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_UP, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);
    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);
}


void neutral_date_skip(ConsoleHandle& console, ProControllerContext& context){
    ConsoleType type = console.state().console_type();

    if (is_switch1(type)){
        switch (context->performance_class()){
        case ControllerPerformanceClass::SerialPABotBase_Wired:
            neutral_date_skip_switch1_wired(context);
            return;
        case ControllerPerformanceClass::SerialPABotBase_Wireless:
            neutral_date_skip_switch1_wireless(context);
            return;
        case ControllerPerformanceClass::SysbotBase:
            neutral_date_skip_switch1_sbb(context);
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
            neutral_date_skip_switch2_wired(context);
            return;
        case ControllerPerformanceClass::SerialPABotBase_Wireless:
            neutral_date_skip_switch2_wireless(context);
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
