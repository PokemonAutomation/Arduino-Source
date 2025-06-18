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
void neutral_date_skip_switch1_wireless(ProControllerContext& context){
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_press_button(context, BUTTON_A, 20, 10);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 0);
    ssf_press_button(context, BUTTON_A, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
//    ssf_press_button(context, BUTTON_A, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_press_button(context, BUTTON_A, 20, 10);
    ssf_press_button(context, BUTTON_A, 20, 10);
    for (uint8_t c = 0; c < 6; c++){
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, unit);
    }
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);
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


void neutral_date_skip(ConsoleHandle& console, ProControllerContext& context){
    ConsoleType type = console.state().console_type();

    if (is_switch1(type)){
        switch (context->performance_class()){
        case ControllerPerformanceClass::SerialPABotBase_Wired_125Hz:
            neutral_date_skip_switch1_wired(context);
            return;
        case ControllerPerformanceClass::SerialPABotBase_Wireless_ESP32:
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
        neutral_date_skip_switch2_wired(context);
        return;
    }

    throw UserSetupError(
        console.logger(),
        "Please select a valid Switch console type."
    );
}





}
}
