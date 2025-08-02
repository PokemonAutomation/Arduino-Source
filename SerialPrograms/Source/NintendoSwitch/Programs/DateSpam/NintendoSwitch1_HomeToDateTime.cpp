/*  Nintendo Switch 2 Home To Date-Time
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/RecursiveThrottler.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
//#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
//#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




void home_to_date_time_Switch1_wired_blind(
    Logger& logger, ProControllerContext& context, bool to_date_change
){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("NintendoSwitch::home_to_date_time_Switch1_wired_blind()");
    }

    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 32ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 32ms);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 32ms);

    //  Down twice in case we drop one.
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 32ms);

    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms, 48ms, 24ms);

    //  Two A presses in case we drop the 1st one.
    ssf_press_button(context, BUTTON_A, 24ms, 40ms, 24ms);
    ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);

    //  Just button mash it. lol
    {
        auto iterations = Milliseconds(1200) / 24ms + 1;
        do{
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
        }while (--iterations);
    }

    //  Scroll left and press A to exit the sleep menu if we happened to
    //  land there.
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 24ms);
    ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);

    {
        auto iterations = Milliseconds(312) / 24ms + 1;
        do{
            ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms);
        }while (--iterations);
    }

    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 80ms, 40ms, 24ms);
    ssf_press_dpad(context, DPAD_DOWN, 360ms, 320ms);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);

    if (!to_date_change){
        //  Triple up this A press to make sure it gets through.
        ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
        ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
        ssf_press_button(context, BUTTON_A, 360ms, 48ms, 24ms);
        return;
    }

    //  Triple up this A press to make sure it gets through.
    ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
    {
        auto iterations = Milliseconds(250) / 24ms + 1;
        do{
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
        }while (--iterations);
    }
//        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);

    //  Left scroll in case we missed and landed in the language change or sleep
    //  confirmation menus.
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms);
}
void home_to_date_time_Switch1_wireless_esp32_blind(
    Logger& logger, ProControllerContext& context, bool to_date_change
){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("NintendoSwitch::home_to_date_time_Switch1_wireless_esp32_blind()");
    }

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
        //  Triple up this A press to make sure it gets through.
        ssf_press_button(context, BUTTON_A, unit);
        ssf_press_button(context, BUTTON_A, unit);
        ssf_press_button(context, BUTTON_A, 360ms, 2*unit, unit);
        return;
    }

    //  Triple up this A press to make sure it gets through.
    ssf_press_button(context, BUTTON_A, unit);
    ssf_press_button(context, BUTTON_A, unit);
    ssf_press_button(context, BUTTON_A, unit);
    {
        auto iterations = Milliseconds(250) / unit + 1;
        do{
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
        }while (--iterations);
    }

    //  Left scroll in case we missed landed in the language change or sleep
    //  confirmation menus.
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms, 2*unit, unit);

}
void home_to_date_time_Switch1_sbb_blind(
    Logger& logger, ProControllerContext& context, bool to_date_change
){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("NintendoSwitch::home_to_date_time_Switch1_sbb_blind()");
    }

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


void home_to_date_time_Switch1_wired_feedback(
    VideoStream& stream, ProControllerContext& context, bool to_date_change
){
    stream.log("home_to_date_time_Switch1_wired_with_feedback()");

    size_t max_attempts = 5;
    for (size_t i = 0; i < max_attempts; i++){
        {
            ThrottleScope scope(context->logging_throttler());
            if (scope){
                context->logger().log("NintendoSwitch::home_to_date_time_Switch1_wired_feedback() - Part 1/3");
            }

            ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 32ms);
            ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 32ms);
            ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 32ms);

            //  Down twice in case we drop one.
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 32ms);

            // if (i > 0){  // intentionally create a failure, for testing
            ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms, 40ms, 24ms);
            // }


            // ImageFloatBox system_icon(0.685, 0.69, 0.05, 0.03);
            // ImageFloatBox other_setting1(0.615, 0.69, 0.05, 0.03);
            // ImageFloatBox other_setting2(0.545, 0.69, 0.05, 0.03);

            //  Two A presses in case we drop the 1st one.
            //  the program can self recover even if the second button press is registered.
            ssf_press_button(context, BUTTON_A, 24ms, 40ms, 24ms);
            ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);

            //  Just button mash it. lol
            {
                auto iterations = Milliseconds(1200) / 24ms + 1;
                do{
                    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
                }while (--iterations);
            }
        }

        context.wait_for_all_requests();
        // Should now be in System Settings, with System highlighted
        ImageFloatBox system_setting_box(0.056, 0.74, 0.01, 0.1);
        ImageFloatBox other_setting1(0.04, 0.74, 0.01, 0.1);
        ImageFloatBox other_setting2(0.02, 0.74, 0.01, 0.1);
        SelectedSettingWatcher system_setting_selected(system_setting_box, other_setting1, other_setting2);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                for (int i = 0; i < 10; i++){
                    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
                }
            },
            {system_setting_selected}
        );
        if (ret < 0){  // failed to detect "System" being highlighted. press home and re-try
            pbf_press_button(context, BUTTON_HOME, 100ms, 2000ms);
            stream.log("home_to_date_time_Switch1_wired_feedback: Failed to detect 'System' being highlighted. Re-try", COLOR_YELLOW);
            continue;
        }

        {
            ThrottleScope scope(context->logging_throttler());
            if (scope){
                context->logger().log("NintendoSwitch::home_to_date_time_Switch1_wired_feedback() - Part 2/3");
            }

            {
                auto iterations = Milliseconds(312) / 24ms + 1;
                do{
                    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 24ms);
                }while (--iterations);
            }

            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 80ms, 40ms, 24ms);
            ssf_press_dpad(context, DPAD_DOWN, 360ms, 320ms);
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
            // if (i > 1){  // intentionally create a failure, for testing
            ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
            // }

            // only one ButtonA press since the program can self-recover if the button is dropped.
            // furthermore, the program can't self-recover if a second button press is registered.
            ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
        }


        context.wait_for_all_requests();
        context.wait_for(Milliseconds(300));
        // we expect to be within "Date and Time", with "Synchronize Clock via Internet" being highlighted
        ImageFloatBox sync_clock_box(0.168, 0.185, 0.01, 0.1);
        ImageFloatBox other_setting3(0.1, 0.185, 0.01, 0.1);
        ImageFloatBox other_setting4(0.05, 0.185, 0.01, 0.1);
        SelectedSettingWatcher sync_clock_selected(sync_clock_box, other_setting3, other_setting4);
        ret = wait_until(
            stream, context,
            Milliseconds(2000),
            {sync_clock_selected}
        );
        if (ret < 0){  // failed to detect "Synchronize clock" being highlighted. press home and re-try
            pbf_press_button(context, BUTTON_HOME, 100ms, 2000ms);
            stream.log("home_to_date_time_Switch1_wired_feedback: Failed to detect 'Synchronize clock' being highlighted. Re-try", COLOR_YELLOW);
            continue;
        }


        if (!to_date_change){
            stream.log("Arrived at Date and Time.", COLOR_ORANGE);
            return;
        }

        ThrottleScope scope(context->logging_throttler());
        if (scope){
            context->logger().log("NintendoSwitch::home_to_date_time_Switch1_wired_feedback() - Part 3/3");
        }


        {
            auto iterations = Milliseconds(250) / 24ms + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
            }while (--iterations);
        }

        //  Left scroll in case we missed landed in the language change or sleep
        //  confirmation menus.
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms);
        stream.log("Arrived at Date and Time, date change.", COLOR_ORANGE);
        return;
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "home_to_date_time(): Failed to reach Date and Time after several attempts.",
        stream
    );

}

void home_to_date_time_Switch1_wireless_esp32_feedback(
    VideoStream& stream, ProControllerContext& context, bool to_date_change
){
    stream.log("home_to_date_time_Switch1_wireless_esp32_feedback()");

    size_t max_attempts = 5;
    for (size_t i = 0; i < max_attempts; i++){
        ThrottleScope scope(context->logging_throttler());
        if (scope){
            context->logger().log("NintendoSwitch::home_to_date_time_Switch1_wireless_esp32_feedback()");
        }

        Milliseconds tv = context->timing_variation();
        Milliseconds unit = 24ms + tv;

        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);

        //  Down twice in case we drop one.
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);

        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms, 2*unit, unit);

        //  Two A presses in case we drop the 1st one.
        //  the program can self recover even if the second button press is registered.
        pbf_press_button(context, BUTTON_A, 2*unit, unit);
        pbf_press_button(context, BUTTON_A, 2*unit, unit);

        //  Just button mash it. lol
        {
            auto iterations = Milliseconds(1100) / unit + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
            }while (--iterations);
        }

        context.wait_for_all_requests();
        // Should now be in System Settings, with System highlighted
        ImageFloatBox system_setting_box(0.056, 0.74, 0.01, 0.1);
        ImageFloatBox other_setting1(0.04, 0.74, 0.01, 0.1);
        ImageFloatBox other_setting2(0.02, 0.74, 0.01, 0.1);
        SelectedSettingWatcher system_setting_selected(system_setting_box, other_setting1, other_setting2);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [&](ProControllerContext& context){
                for (int i = 0; i < 10; i++){
                    ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
                }
            },
            {system_setting_selected}
        );
        if (ret < 0){  // failed to detect "System" being highlighted. press home and re-try
            pbf_press_button(context, BUTTON_HOME, 100ms, 2000ms);
            stream.log("home_to_date_time_Switch1_wireless_esp32_feedback: Failed to detect 'System' being highlighted. Re-try", COLOR_YELLOW);
            continue;
        }        

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
        // if (i > 1){  // intentionally create a failure, for testing
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
        // }

        // only one ButtonA press since the program can self-recover if the button is dropped.
        // furthermore, the program can't self-recover if a second button press is registered.
        ssf_press_button(context, BUTTON_A, 360ms, 2*unit, unit);

        context.wait_for_all_requests();
        context.wait_for(Milliseconds(300));
        // we expect to be within "Date and Time", with "Synchronize Clock via Internet" being highlighted
        ImageFloatBox sync_clock_box(0.168, 0.185, 0.01, 0.1);
        ImageFloatBox other_setting3(0.1, 0.185, 0.01, 0.1);
        ImageFloatBox other_setting4(0.05, 0.185, 0.01, 0.1);
        SelectedSettingWatcher sync_clock_selected(sync_clock_box, other_setting3, other_setting4);
        ret = wait_until(
            stream, context,
            Milliseconds(2000),
            {sync_clock_selected}
        );
        if (ret < 0){  // failed to detect "Synchronize clock" being highlighted. press home and re-try
            pbf_press_button(context, BUTTON_HOME, 100ms, 2000ms);
            stream.log("home_to_date_time_Switch1_wireless_esp32_feedback: Failed to detect 'Synchronize clock' being highlighted. Re-try", COLOR_YELLOW);
            continue;
        }


        if (!to_date_change){
            stream.log("Arrived at Date and Time.", COLOR_ORANGE);
            return;
        }        


        {
            auto iterations = Milliseconds(250) / unit + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
            }while (--iterations);
        }

        //  Left scroll in case we missed landed in the language change or sleep
        //  confirmation menus.
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms, 2*unit, unit);
        stream.log("Arrived at Date and Time, date change.", COLOR_ORANGE);
        return;
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "home_to_date_time(): Failed to reach Date and Time after several attempts.",
        stream
    );    

}




void home_to_date_time_Switch1_joycon_blind(JoyconContext& context, bool to_date_change){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("NintendoSwitch::home_to_date_time_Switch1_joycon_blind()");
    }

    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 100ms + tv;

    //From ControllerPerformanceClass::SerialPABotBase_Wireless
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
