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
namespace Switch1{




void init_view(ProControllerContext& context){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch1::init_view()");
    }
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 160ms, 80ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 160ms, 80ms);
}
void auto_recovery(ProControllerContext& context){
    //  Reset the state of the Date/Time menu to unsynced with cursor
    //  over the "Date and Time" option.

    //  Merge 3 different states. The 1st column is the error-free state.
    //  There are more states than this, but the normal skipping loop will
    //  eventually force them all into one of these 3 states.

    //  Cost: 161 cycles

    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::auto_recovery()");
    }

    //  menu-ok-2   menu-ok-0   menu-sy-0
    for (uint8_t c = 0; c < 13; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms, 48ms, 24ms);
    }
//    pbf_wait(500);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0ms, 48ms, 24ms);

    //  menu-ok-2   menu-ok-2   menu-sy-1
    ssf_press_button(context, BUTTON_A, 320ms, 80ms);
    //  date-ok     date-ok     zone-sy
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms, 48ms, 24ms);
    //  date-mn     date-mn     zone-sy
    ssf_press_button(context, BUTTON_A, 320ms, 80ms);
    //  date-ok     date-ok     menu-sy-1
    ssf_issue_scroll(context, SSF_SCROLL_UP, 0ms, 48ms, 24ms);
    //  date-ok     date-ok     menu-sy-0
    ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
    //  menu-ok-2   menu-ok-2   menu-ok-0
    for (uint8_t c = 0; c < 13; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms, 48ms, 24ms);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0ms, 48ms, 24ms);
    //  menu-ok-2   menu-ok-2   menu-ok-2
//    pbf_wait(500);
}
void increment_day_with_feedback(VideoStream& stream, ProControllerContext& context, bool date_us){
    //  60 cycles (robust 2)

    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch1::increment_day_with_feedback()");
    }

    size_t max_attempts = 5;
    for (size_t i = 0; i < max_attempts; i++){

    // ImageFloatBox date_box(0.30, 0.705, 0.1, 0.02);
    // ImageFloatBox other_box1(0.30, 0.4, 0.1, 0.02);
    // ImageFloatBox other_box2(0.30, 0.48, 0.1, 0.02);

    context.wait_for_all_requests();
    // context.wait_for(Milliseconds(10000));
    // we expect Date and Time to be highlighted
    ImageFloatBox date_time_box(0.50, 0.58, 0.1, 0.02);
    // ImageFloatBox date_time_box(0.166, 0.495, 0.01, 0.08);
    ImageFloatBox other_box1(0.01, 0.7, 0.1, 0.02);
    ImageFloatBox other_box2(0.01, 0.75, 0.1, 0.02);
    SelectedSettingWatcher date_time_selected(date_time_box, other_box1, other_box2);
    int ret = wait_until(
        stream, context,
        Milliseconds(1000),
        {date_time_selected}
    );
    if (ret < 0){  // failed to detect "Date and Time" being highlighted. 
        auto_recovery(context);
        continue;
    }

    //  Press A twice. If the 1st one gets dropped due to the exit
    //  animation taking too long, the 2nd press will recover to
    //  avoid the trapping error at the cost of losing the skip.
    // if (i > 0){   // intentionally create a failure, for testing
    ssf_press_button(context, BUTTON_A, 120ms);
    ssf_press_button(context, BUTTON_A, 40ms);
    // }

    if (date_us){
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms);
    }
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP, 0ms, 48ms, 24ms);

    ssf_press_button(context, BUTTON_A, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0ms, 48ms, 24ms);

    //  Press A twice here to make sure we exit the window. If the 1st one is
    //  dropped, we'll be slow to exit, but the 2nd A press of the next
    //  iteration will correct this as there will be more than 20 ticks.
    ssf_press_button(context, BUTTON_A, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms);  //  Extra right presses to make sure we land on OK.
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms);
    ssf_press_button(context, BUTTON_A, 96ms);
    return;
    }
}

void increment_day(ProControllerContext& context, bool date_us){
    //  60 cycles (robust 2)

    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch1::increment_day()");
    }

    //  Press A twice. If the 1st one gets dropped due to the exit
    //  animation taking too long, the 2nd press will recover to
    //  avoid the trapping error at the cost of losing the skip.
    ssf_press_button(context, BUTTON_A, 120ms);
    ssf_press_button(context, BUTTON_A, 40ms);

    if (date_us){
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    }
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP,    0ms, 48ms, 24ms);

    ssf_press_button(context, BUTTON_A, 16ms, 40ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT,  0ms, 48ms, 24ms);

    //  Press A twice here to make sure we exit the window. If the 1st one is
    //  dropped, we'll be slow to exit, but the 2nd A press of the next
    //  iteration will correct this as there will be more than 20 ticks.
    ssf_press_button(context, BUTTON_A, 24ms, 40ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);  //  Extra right presses to make sure we land on OK.
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 16ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 96ms);
}
void rollback_year_full(ProControllerContext& context, bool date_us){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch1::rollback_year_full()");
    }

    ssf_press_button(context, BUTTON_A, 96ms);
    ssf_press_button(context, BUTTON_A, 64ms);

    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    if (date_us){
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    }

    for (uint8_t c = 0; c < 60; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms, 48ms, 24ms);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0ms, 48ms, 24ms);

    ssf_press_button(context, BUTTON_A, 16ms, 40ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT,  0ms, 48ms, 24ms);

    //  Press A twice here to make sure we exit the window. If the 1st one is
    //  dropped, we'll be slow to exit, but the 2nd A press of the next
    //  iteration will correct this as there will be a full 24 ticks.
    ssf_press_button(context, BUTTON_A, 24ms, 40ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);  //  Extra right presses to make sure we land on OK.
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 16ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 96ms);
}
void rollback_year_sync(ProControllerContext& context){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch1::rollback_year_sync()");
    }

//    pbf_wait(500);

    //  We cannot enable time-sync too soon or the date change will not
    //  register. This mean that an artificial delay is needed which slows
    //  down the overall speed.

    //  While we're waiting, we might as well spam up.
    for (uint8_t c = 0; c < 13; c++){
        ssf_issue_scroll(context, SSF_SCROLL_UP, 24ms, 48ms, 24ms);
    }
    ssf_issue_scroll(context, SSF_SCROLL_UP, 0ms, 48ms, 24ms);

//    pbf_wait(36);

    ssf_press_button(context, BUTTON_A, 80ms);
    ssf_press_button(context, BUTTON_A, 120ms);

    //  This is one more iteration than is needed for safety.
    for (uint8_t c = 0; c < 10; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms, 48ms, 24ms);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0ms, 48ms, 24ms);

//    pbf_wait(500);
}
void increment_monthday(ProControllerContext& context){
    //  69 cycles

    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch1::increment_monthday()");
    }

    //  Press A twice. If the 1st one gets dropped due to the exit
    //  animation taking too long, the 2nd press will recover to
    //  avoid the trapping error at the cost of losing the skip.
    ssf_press_button(context, BUTTON_A, 120ms);
    ssf_press_button(context, BUTTON_A, 40ms);

    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP,    0ms, 48ms, 24ms);

    ssf_press_button(context, BUTTON_A, 64ms);      //  Seems to work at 7.
    ssf_issue_scroll(context, SSF_SCROLL_UP, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT,  0ms, 48ms, 24ms);

    //  Press A twice here to make sure we exit the window. If the 1st one is
    //  dropped, we'll be slow to exit, but the 2nd A press of the next
    //  iteration will correct this as there will be more than 20 ticks.
    ssf_press_button(context, BUTTON_A, 24ms, 40ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);  //  Extra right presses to make sure we land on OK.
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 16ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 96ms);
}
void increment_daymonth(ProControllerContext& context){
    //  64 cycles

    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch1::increment_daymonth()");
    }

    //  Press A twice. If the 1st one gets dropped due to the exit
    //  animation taking too long, the 2nd press will recover to
    //  avoid the trapping error at the cost of losing the skip.
    ssf_press_button(context, BUTTON_A, 96ms);
    ssf_press_button(context, BUTTON_A, 64ms);

    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP,   24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP,   0ms, 48ms, 24ms);

    ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0ms, 48ms, 24ms);

    //  Press A twice here to make sure we exit the window. If the 1st one is
    //  dropped, we'll be slow to exit, but the 2nd A press of the next
    //  iteration will correct this as there will be more than 20 ticks.
    ssf_press_button(context, BUTTON_A, 24ms, 40ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms); //  Extra right presses to make sure we land on OK.
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 16ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 96ms);
}
void increment_month(ProControllerContext& context, uint8_t days){
    if (days < 31){
        increment_monthday(context);
    }else{
        increment_daymonth(context);
    }
}
void increment_all(ProControllerContext& context){
    //  72 cycles

    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch1::increment_all()");
    }

    //  Press A twice. If the 1st one gets dropped due to the exit
    //  animation taking too long, the 2nd press will recover to
    //  avoid the trapping error at the cost of losing the skip.
    ssf_press_button(context, BUTTON_A, 96ms);
    ssf_press_button(context, BUTTON_A, 64ms);

    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP,   24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP,   24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP,    0ms, 48ms, 24ms);

    ssf_press_button(context, BUTTON_A, 16ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT,  0ms, 48ms, 24ms);

    //  Press A twice here to make sure we exit the window. If the 1st one is
    //  dropped, we'll be slow to exit, but the 2nd A press of the next
    //  iteration will correct this as there will be more than 20 ticks.
    ssf_press_button(context, BUTTON_A, 24ms, 40ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms); //  Extra right presses to make sure we land on OK.
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 16ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 96ms);
}
void increment_all_rollback(ProControllerContext& context){
    //  249 cycles

    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("DateSkippers::Switch1::increment_all_rollback()");
    }

    //  Press A twice. If the 1st one gets dropped due to the exit
    //  animation taking too long, the 2nd press will recover to
    //  avoid the trapping error at the cost of losing the skip.
    ssf_press_button(context, BUTTON_A, 96ms);
    ssf_press_button(context, BUTTON_A, 64ms);

    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP,   24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_UP,   24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms, 48ms, 24ms);
    for (uint8_t c = 60; c > 1; c--){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms, 48ms, 24ms);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN,   0ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 16ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0ms, 48ms, 24ms);

    //  Press A twice here to make sure we exit the window. If the 1st one is
    //  dropped, we'll be slow to exit, but the 2nd A press of the next
    //  iteration will correct this as there will be more than 20 ticks.
    ssf_press_button(context, BUTTON_A, 24ms, 40ms, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms, 48ms, 24ms);  //  Extra right presses to make sure we land on OK.
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 16ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 96ms);
}




}
}
}
}
