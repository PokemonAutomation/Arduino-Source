/*  Nintendo Switch Home To Date-Time
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
//#include "CommonFramework/Exceptions/OperationFailedException.h"
//#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
//#include "CommonTools/Async/InferenceRoutines.h"
#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
//#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
//#include "NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.h"
#include "NintendoSwitch_HomeToDateTime.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




void home_to_date_time_Switch1_blind(Logger& logger, ProControllerContext& context, bool to_date_change){
    switch (context->performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired_125Hz:
        home_to_date_time_Switch1_wired_blind(logger, context, to_date_change);
        return;
    case ControllerPerformanceClass::SerialPABotBase_Wireless_ESP32:
        home_to_date_time_Switch1_wireless_esp32_blind(logger, context, to_date_change);
        return;
    default:
        //  Slow version for tick-imprecise controllers.
        home_to_date_time_Switch1_sbb_blind(logger, context, to_date_change);
        return;
    }
}
bool home_to_date_time_Switch1_feedback(ConsoleHandle& console, ProControllerContext& context, bool to_date_change){
    switch (context->performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired_125Hz:
        home_to_date_time_Switch1_wired_feedback(console, context, to_date_change);
        return true;
    default:;
        return false;
    }
}


void home_to_date_time_Switch2_blind(
    Logger& logger, ProControllerContext& context,
    ConsoleType console_type, bool to_date_change
){
    switch (context->performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired_125Hz:
        home_to_date_time_Switch2_wired_blind(logger, context, console_type, to_date_change);
        return;
    case ControllerPerformanceClass::SerialPABotBase_Wireless_ESP32:
        home_to_date_time_Switch2_wireless_blind(logger, context, console_type, to_date_change);
        return;
    default:
        //  Slow version for tick-imprecise controllers.
        home_to_date_time_Switch1_sbb_blind(logger, context, to_date_change);
        return;
    }
}
bool home_to_date_time_Switch2_feedback(
    ConsoleHandle& console, ProControllerContext& context,
    ConsoleType console_type, bool to_date_change
){
    switch (context->performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired_125Hz:
        home_to_date_time_Switch2_wired_feedback(console, context, to_date_change);
        return true;
    case ControllerPerformanceClass::SerialPABotBase_Wireless_ESP32:
        home_to_date_time_Switch2_wireless_feedback(console, context, to_date_change);
        return true;
    default:;
        return false;
    }
}




//  Returns true if success. False if not supported.
bool home_to_date_time_with_feedback(ConsoleHandle& console, ProControllerContext& context, bool to_date_change){
    ensure_at_home(console, context);

    ConsoleTypeDetector_Home detector(console);
    ConsoleType console_type = detector.detect_only(console.video().snapshot());
    switch (console_type){
    case ConsoleType::Switch1:
        return home_to_date_time_Switch1_feedback(console, context, to_date_change);
    case ConsoleType::Switch2_Unknown:
    case ConsoleType::Switch2_FW19_International:
    case ConsoleType::Switch2_FW19_JapanLocked:
    case ConsoleType::Switch2_FW20_International:
    case ConsoleType::Switch2_FW20_JapanLocked:
        home_to_date_time_Switch2_feedback(console, context, console_type, to_date_change);
        return true;
    default:;
    }

    return false;
}
void home_to_date_time(ConsoleHandle& console, ProControllerContext& context, bool to_date_change){
    if (console.video().snapshot() && home_to_date_time_with_feedback(console, context, to_date_change)){
        return;
    }

    //  No feedback available.

    ConsoleType console_type = console.state().console_type();

    switch (console_type){
    case ConsoleType::Unknown:
        throw UserSetupError(console, "Switch type is not specified and feedback is not available.");
    case ConsoleType::Switch1:
        home_to_date_time_Switch1_blind(console, context, to_date_change);
        return;
    case ConsoleType::Switch2_Unknown:
    case ConsoleType::Switch2_FW19_International:
    case ConsoleType::Switch2_FW19_JapanLocked:
    case ConsoleType::Switch2_FW20_International:
    case ConsoleType::Switch2_FW20_JapanLocked:
        home_to_date_time_Switch2_blind(console, context, console_type, to_date_change);
        return;
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
