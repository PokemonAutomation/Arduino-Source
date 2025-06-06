/*  Nintendo Switch Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Controllers/ControllerTypes.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch_Navigation.h"

#include <cmath>

namespace PokemonAutomation{
namespace NintendoSwitch{


bool is_white_theme(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    VideoSnapshot snapshot = stream.video().snapshot();
    ImageFloatBox window_top(0.60, 0.02, 0.35, 0.05);
    ImageStats stats_window_top = image_stats(extract_box_reference(snapshot, window_top));
    bool white_theme = stats_window_top.average.sum() > 600;
    return white_theme;
}

bool is_setting_selected(VideoStream& stream, ProControllerContext& context, ImageFloatBox selected_box, ImageFloatBox unselected_box1, ImageFloatBox unselected_box2){
    VideoOverlaySet overlays(stream.overlay());
    overlays.add(COLOR_RED, selected_box);
    overlays.add(COLOR_BLUE, unselected_box1);
    overlays.add(COLOR_BLUE, unselected_box2);
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(250));
    size_t max_attempts = 10;  // multiple attempts because the highlighted icon/setting pulses. and sometimes there isn't enough contrast at the exact moment you take the snapshot.
    bool is_selected = false;
    for (size_t i = 0; i < max_attempts; i++){
        VideoSnapshot snapshot = stream.video().snapshot();

        ImageStats stats_unselected_box1 = image_stats(extract_box_reference(snapshot, unselected_box1));
        double unselected1_average_sum = stats_unselected_box1.average.sum();
        stream.log("unselected_average_sum1: " + std::to_string(unselected1_average_sum));

        ImageStats stats_unselected_box2 = image_stats(extract_box_reference(snapshot, unselected_box2));
        double unselected2_average_sum = stats_unselected_box2.average.sum();
        stream.log("unselected_average_sum2: " + std::to_string(unselected2_average_sum));

        double average_sum_unselected_diff = std::abs(unselected1_average_sum - unselected2_average_sum);

        ImageStats stats_selected_box = image_stats(extract_box_reference(snapshot, selected_box));
        double selected_average_sum = stats_selected_box.average.sum();
        stream.log("selected_average_sum: " + std::to_string(selected_average_sum));

        if (is_white_theme(stream, context)){  // light mode
            // unselected should be brighter than selected
            is_selected = selected_average_sum < std::min(unselected1_average_sum, unselected2_average_sum) - average_sum_unselected_diff - 20 ;
        }else{  // dark mode
            // selected should be brighter than unselected
            is_selected = selected_average_sum  > std::max(unselected1_average_sum, unselected2_average_sum) + average_sum_unselected_diff + 20;
        }

        if(is_selected){
            return true;
        }
        context.wait_for(Milliseconds(100));
    }

    return false;
}

void home_to_date_time(VideoStream& stream, ProControllerContext& context, bool to_date_change, bool fast){
    size_t max_attempts = 5;
    for (size_t i = 0; i < max_attempts; i++){
    switch (context->performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired_125Hz:{
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 4);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 4);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 4);

        //  Down twice in case we drop one.
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 3);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 4);

        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0);

        // ImageFloatBox system_icon(0.685, 0.69, 0.05, 0.03);
        // ImageFloatBox other_setting1(0.615, 0.69, 0.05, 0.03);
        // ImageFloatBox other_setting2(0.545, 0.69, 0.05, 0.03);

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

        // Should now be in System Settings, with System highlighted
        ImageFloatBox system_setting_box(0.056, 0.74, 0.01, 0.1);
        ImageFloatBox other_setting1(0.04, 0.74, 0.01, 0.1);
        ImageFloatBox other_setting2(0.02, 0.74, 0.01, 0.1);
        SelectedSettingWatcher system_setting(system_setting_box, other_setting1, other_setting2);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                for (int i = 0; i < 10; i++){
                    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
                }
            },
            {system_setting}
        );
        if (ret < 0){  // failed to detect System highlighted. press home and re-try
            pbf_press_button(context, BUTTON_HOME, 100ms, 100ms);
            continue;
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


        //  double up this A press to make sure it gets through.
        ssf_press_button(context, BUTTON_A, 3);
        ssf_press_button(context, BUTTON_A, 3);

        context.wait_for_all_requests();
        // we expect to be within "Date and Time", with "Synchronize Clock via Internet" being highlighted
        ImageFloatBox sync_clock_box(0.168, 0.185, 0.01, 0.1);
        ImageFloatBox other_setting3(0.1, 0.185, 0.01, 0.1);
        ImageFloatBox other_setting4(0.05, 0.185, 0.01, 0.1);
        SelectedSettingWatcher sync_clock(sync_clock_box, other_setting3, other_setting4);
        ret = wait_until(
            stream, context,
            Milliseconds(2000),
            {sync_clock}
        );
        if (ret < 0){  // failed to detect System highlighted. press home and re-try
            pbf_press_button(context, BUTTON_HOME, 100ms, 100ms);
            continue;
        }


        if (!to_date_change){
            return;
        }

        {
            auto iterations = Milliseconds(250) / 24ms + 1;
            do{
                ssf_issue_scroll(context, SSF_SCROLL_DOWN, 24ms);
            }while (--iterations);
        }
//        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 0);

        //  Left scroll in case we missed landed in the language change or sleep
        //  confirmation menus.
        ssf_issue_scroll(context, SSF_SCROLL_LEFT, 0ms);

        return;
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

        return;
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
        return;
    }
    }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "home_to_date_time(): Failed to reach Date and Time after several attempts.",
        stream
    );

}




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
            //  Triple up this A press to make sure it gets through.
            ssf_press_button(context, BUTTON_A, 3);
            ssf_press_button(context, BUTTON_A, 3);
            ssf_press_button(context, BUTTON_A, 45);
            return;
        }

        //  Triple up this A press to make sure it gets through.
        ssf_press_button(context, BUTTON_A, 3);
        ssf_press_button(context, BUTTON_A, 3);
        ssf_press_button(context, BUTTON_A, 3);
        {
            auto iterations = Milliseconds(250) / 24ms + 1;
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
