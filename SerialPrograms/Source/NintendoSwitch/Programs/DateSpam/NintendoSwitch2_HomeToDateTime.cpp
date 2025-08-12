/*  Nintendo Switch 2 Home To Date-Time
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/RecursiveThrottler.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch2_BinarySliderDetector.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



template <typename ControllerContext>
ConsoleType settings_detect_console_type(
    ConsoleHandle& console, ControllerContext& context
){
    ConsoleType console_type = console.state().console_type();
    console.log(std::string("Console Type: ") + ConsoleType_strings(console_type));

    //  We already know for sure the Switch type.
    if (console_type != ConsoleType::Switch2_Unknown &&
        console.state().console_type_confirmed()
    ){
        return console_type;
    }

    console.log("Console type unknown or not confirmed. Attempting to detect...");

    VideoOverlaySet overlays(console.overlay());
    BinarySliderDetector detector(COLOR_BLUE, {0.836431, 0.097521, 0.069703, 0.796694});
    detector.make_overlays(overlays);

    ssf_do_nothing(context, 500ms);
    for (size_t c = 0; c < 6; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 200ms, 100ms, 100ms);
    }
    ssf_do_nothing(context, 500ms);
    context.wait_for_all_requests();

    auto snapshot = console.video().snapshot();
//    snapshot->save("screenshot.png");
    size_t sliders = detector.detect(snapshot).size();
    switch (sliders){
    case 1:
    case 2:
        console.state().set_console_type(console, ConsoleType::Switch2_FW20_International);
        break;
    case 3:
        console.state().set_console_type(console, ConsoleType::Switch2_FW20_JapanLocked);
        break;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to detect if this Switch 2 model is international or Japan-locked.",
            console, std::move(snapshot)
        );
    }
    console_type = console.state().console_type();
    console.log(std::string("Detected console type as: ") + ConsoleType_strings(console_type));

    //  Scroll back up.
    for (size_t c = 0; c < 7; c++){
        ssf_issue_scroll(context, SSF_SCROLL_UP, 200ms, 100ms, 100ms);
    }
    ssf_do_nothing(context, 500ms);

    return console_type;
}






void home_to_settings_Switch2_procon_blind(
    ProControllerContext& context
){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("NintendoSwitch::home_to_settings_Switch2_procon_blind()");
    }

    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);

    //  Down twice in case we drop one.
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);

    ssf_issue_scroll(context, SSF_SCROLL_LEFT, unit);

    //  Two A presses in case we drop the 1st one.
    ssf_press_button(context, BUTTON_A, unit);
    ssf_press_button(context, BUTTON_A, unit);

    for (size_t c = 0; c < 40; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 1000ms, 1000ms, unit);

    //  Scroll left and press A to exit the sleep menu if we happened to
    //  land there.
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, unit);
    ssf_press_button(context, BUTTON_A, unit);

    for (size_t c = 0; c < 2; c++){
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, unit);
    }
}
void home_to_settings_Switch2_joycon_blind(
    JoyconContext& context
){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("NintendoSwitch::home_to_settings_Switch2_joycon_blind()");
    }

    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

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

    pbf_move_joystick(context, 128, 255, 2000ms, 0ms);

    //  Scroll left and press A to exit the sleep menu if we happened to
    //  land there.
    pbf_move_joystick(context, 0, 128, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);

    for (size_t c = 0; c < 2; c++){
        pbf_move_joystick(context, 255, 128, 2*unit, unit);
    }
}


template <typename ControllerContext>
void settings_to_date_time_Switch2_all_blind(
    Logger& logger, ControllerContext& context,
    ConsoleType console_type, bool to_date_change
){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("NintendoSwitch::settings_to_date_time_Switch2_all_blind()");
    }

    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 192ms, 2*unit, unit);

    switch (console_type){
    case ConsoleType::Switch2_FW19_International:
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, 2*unit, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, 2*unit, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, 2*unit, unit);
        break;
    case ConsoleType::Switch2_FW19_JapanLocked:
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, 2*unit, unit);
        break;
    case ConsoleType::Switch2_FW20_International:
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, 2*unit, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, 2*unit, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, 2*unit, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, 2*unit, unit);
        break;
    case ConsoleType::Switch2_FW20_JapanLocked:
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, 2*unit, unit);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, 2*unit, unit);
        break;
    default:
        throw UserSetupError(
            logger,
            "You need to specify a specific Switch 2 model."
        );
    }

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

    for (size_t c = 0; c < 5; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit);
    }

    //  Left scroll in case we missed and landed in the language change or sleep
    //  confirmation menus.
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, unit);
}








void home_to_date_time_Switch2_procon_blind(
    Logger& logger, ProControllerContext& context,
    ConsoleType console_type, bool to_date_change
){
    logger.log("home_to_date_time_Switch2_procon_blind()");
    home_to_settings_Switch2_procon_blind(context);
    settings_to_date_time_Switch2_all_blind(logger, context, console_type, to_date_change);
}
void home_to_date_time_Switch2_procon_feedback(
    ConsoleHandle& console, ProControllerContext& context,
    bool to_date_change
){
    console.log("home_to_date_time_Switch2_procon_feedback()");
    home_to_settings_Switch2_procon_blind(context);
    ConsoleType console_type = settings_detect_console_type(console, context);
    settings_to_date_time_Switch2_all_blind(console, context, console_type, to_date_change);
}





void home_to_date_time_Switch2_joycon_feedback(
    ConsoleHandle& console, JoyconContext& context,
    bool to_date_change
){
    console.log("home_to_date_time_Switch2_joycon_feedback()");
    home_to_settings_Switch2_joycon_blind(context);
    ConsoleType console_type = settings_detect_console_type(console, context);
    settings_to_date_time_Switch2_all_blind(console, context, console_type, to_date_change);
}









}
}
