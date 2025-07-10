/*  Nintendo Switch 2 Home To Date-Time
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch2_BinarySliderDetector.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void home_to_settings_Switch2_wired_blind(
    ProControllerContext& context
){
    Milliseconds delay = 24ms;
    Milliseconds hold = 48ms;
    Milliseconds cool = 24ms;

    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay, hold, cool);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay, hold, cool);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay, hold, cool);

    //  Down twice in case we drop one.
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, delay, hold, cool);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, delay, hold, cool);

    ssf_issue_scroll(context, SSF_SCROLL_LEFT, delay, hold, cool);

    //  Two A presses in case we drop the 1st one.
    ssf_press_button(context, BUTTON_A, delay, hold, cool);
    ssf_press_button(context, BUTTON_A, delay, hold, cool);

    for (size_t c = 0; c < 40; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, delay, hold, cool);
    }
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 1000ms, 1000ms, cool);

    //  Scroll left and press A to exit the sleep menu if we happened to
    //  land there.
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, delay, hold, cool);
    ssf_press_button(context, BUTTON_A, delay, hold, cool);

    for (size_t c = 0; c < 2; c++){
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, delay, hold, cool);
    }
}
void home_to_settings_Switch2_wireless_blind(
    ProControllerContext& context
){
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


void settings_to_date_time_Switch2_wired_blind(
    Logger& logger, ProControllerContext& context,
    ConsoleType console_type, bool to_date_change
){
    Milliseconds delay = 24ms;
    Milliseconds hold = 48ms;
    Milliseconds cool = 24ms;

    ssf_issue_scroll(context, SSF_SCROLL_DOWN, delay, hold, cool);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, delay, hold, cool);
    ssf_issue_scroll(context, SSF_SCROLL_DOWN, 192ms, hold, cool);


    switch (console_type){
    case ConsoleType::Switch2_FW19_International:
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, hold, cool);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, hold, cool);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, hold, cool);
        break;
    case ConsoleType::Switch2_FW19_JapanLocked:
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, hold, cool);
        break;
    case ConsoleType::Switch2_FW20_International:
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, hold, cool);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, hold, cool);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, hold, cool);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, hold, cool);
        break;
    case ConsoleType::Switch2_FW20_JapanLocked:
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, hold, cool);
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 128ms, hold, cool);
        break;
    default:
        throw UserSetupError(
            logger,
            "You need to specify a specific Switch 2 model."
        );
    }


    if (!to_date_change){
        //  Triple up this A press to make sure it gets through.
        ssf_press_button(context, BUTTON_A, delay, hold, cool);
        ssf_press_button(context, BUTTON_A, delay, hold, cool);
        ssf_press_button(context, BUTTON_A, 360ms, hold, cool);
        return;
    }

    //  Triple up this A press to make sure it gets through.
    ssf_press_button(context, BUTTON_A, delay, hold, cool);
    ssf_press_button(context, BUTTON_A, delay, hold, cool);
    ssf_press_button(context, BUTTON_A, delay, hold, cool);

    for (size_t c = 0; c < 5; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, delay, hold, cool);
    }

    //  Left scroll in case we missed and landed in the language change or sleep
    //  confirmation menus.
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, delay, hold, cool);
}
void settings_to_date_time_Switch2_wireless_blind(
    Logger& logger, ProControllerContext& context,
    ConsoleType console_type, bool to_date_change
){
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
        ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
        ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
        ssf_press_button(context, BUTTON_A, 360ms, 48ms, 24ms);
        return;
    }

    //  Triple up this A press to make sure it gets through.
    ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 24ms, 48ms, 24ms);

    for (size_t c = 0; c < 5; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, unit, 2*unit, unit);
    }

    //  Left scroll in case we missed and landed in the language change or sleep
    //  confirmation menus.
    ssf_issue_scroll(context, SSF_SCROLL_LEFT, unit, 2*unit, unit);
}











ConsoleType settings_detect_console_type(
    ConsoleHandle& console, ProControllerContext& context
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
    for (size_t c = 0; c < 5; c++){
        ssf_issue_scroll(context, SSF_SCROLL_DOWN, 200ms, 100ms, 100ms);
    }
    ssf_do_nothing(context, 500ms);
    context.wait_for_all_requests();

    auto snapshot = console.video().snapshot();
//    snapshot->save("screenshot.png");
    size_t sliders = detector.detect(snapshot).size();
    switch (sliders){
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
    for (size_t c = 0; c < 6; c++){
        ssf_issue_scroll(context, SSF_SCROLL_UP, 200ms, 100ms, 100ms);
    }
    ssf_do_nothing(context, 500ms);

    return console_type;
}

void home_to_date_time_Switch2_wired_blind(
    Logger& logger, ProControllerContext& context,
    ConsoleType console_type, bool to_date_change
){
    logger.log("home_to_date_time_Switch2_wired_blind()");
    home_to_settings_Switch2_wired_blind(context);
    settings_to_date_time_Switch2_wired_blind(logger, context, console_type, to_date_change);
}
void home_to_date_time_Switch2_wireless_blind(
    Logger& logger, ProControllerContext& context,
    ConsoleType console_type, bool to_date_change
){
    logger.log("home_to_date_time_Switch2_wireless_blind()");
    home_to_settings_Switch2_wireless_blind(context);
    settings_to_date_time_Switch2_wireless_blind(logger, context, console_type, to_date_change);
}



void home_to_date_time_Switch2_wired_feedback(
    ConsoleHandle& console, ProControllerContext& context,
    bool to_date_change
){
    console.log("home_to_date_time_Switch2_wired_feedback()");
    home_to_settings_Switch2_wired_blind(context);
    ConsoleType console_type = settings_detect_console_type(console, context);
    settings_to_date_time_Switch2_wired_blind(console, context, console_type, to_date_change);
}
void home_to_date_time_Switch2_wireless_feedback(
    ConsoleHandle& console, ProControllerContext& context,
    bool to_date_change
){
    console.log("home_to_date_time_Switch2_wireless_feedback()");
    home_to_settings_Switch2_wireless_blind(context);
    ConsoleType console_type = settings_detect_console_type(console, context);
    settings_to_date_time_Switch2_wireless_blind(console, context, console_type, to_date_change);
}



}
}
