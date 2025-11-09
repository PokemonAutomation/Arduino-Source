/*  Date Skippers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/RecursiveThrottler.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.h"
#include "NintendoSwitch_DateSkippers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace DateSkippers{
namespace Switch2{


void init_view(ProControllerContext& context){
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, unit);
    ssf_press_button(context, BUTTON_A, 256ms + tv, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_press_button(context, BUTTON_A, 264ms + tv, 80ms);
}


void increment_day_us(ProControllerContext& context){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch2::increment_day_us()");
    }

    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_press_button(context, BUTTON_A, 208ms + tv, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_UP, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_press_button(context, BUTTON_A, 264ms + tv, 80ms);
}
void increment_day_eu(ProControllerContext& context){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch2::increment_day_us()");
    }

    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_press_button(context, BUTTON_A, 208ms + tv, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_press_button(context, BUTTON_A, 264ms + tv, 80ms);
}
void increment_day_jp(ProControllerContext& context){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch2::increment_day_us()");
    }

    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_press_button(context, BUTTON_A, 208ms + tv, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_UP, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_press_button(context, BUTTON_A, 264ms + tv, 80ms);
}




}
}
}
}
