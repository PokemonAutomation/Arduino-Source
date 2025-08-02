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
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
//#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
//#include "NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.h"
#include "NintendoSwitch_HomeToDateTime.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




void home_to_date_time_Switch1_blind(Logger& logger, ProControllerContext& context, bool to_date_change){
    switch (context->performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired:
        home_to_date_time_Switch1_wired_blind(logger, context, to_date_change);
        return;
    case ControllerPerformanceClass::SerialPABotBase_Wireless:
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
    case ControllerPerformanceClass::SerialPABotBase_Wired:
        home_to_date_time_Switch1_wired_feedback(console, context, to_date_change);
        return true;
    case ControllerPerformanceClass::SerialPABotBase_Wireless:
        home_to_date_time_Switch1_wireless_esp32_feedback(console, context, to_date_change);
        return true;
    default:
        return false;
    }
}


void home_to_date_time_Switch2_blind(
    Logger& logger, ProControllerContext& context,
    ConsoleType console_type, bool to_date_change
){
    switch (context->performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired:
    case ControllerPerformanceClass::SerialPABotBase_Wireless:
        home_to_date_time_Switch2_procon_blind(logger, context, console_type, to_date_change);
        return;
    default:
        throw InternalProgramError(
            &logger, PA_CURRENT_FUNCTION,
            "Unsupported ControllerPerformanceClass: " + std::to_string((int)context->performance_class())
        );
    }
}
bool home_to_date_time_Switch2_feedback(
    ConsoleHandle& console, ProControllerContext& context,
    ConsoleType console_type, bool to_date_change
){
    switch (context->performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired:
    case ControllerPerformanceClass::SerialPABotBase_Wireless:
        home_to_date_time_Switch2_procon_feedback(console, context, to_date_change);
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
















void home_to_date_time(ConsoleHandle& console, JoyconContext& context, bool to_date_change){
    ensure_at_home(console, context);

    ConsoleTypeDetector_Home detector(console);
    ConsoleType console_type = detector.detect_only(console.video().snapshot());
    switch (console_type){
    case ConsoleType::Switch1:
        home_to_date_time_Switch1_joycon_blind(context, to_date_change);
        return;
    case ConsoleType::Switch2_Unknown:
    case ConsoleType::Switch2_FW19_International:
    case ConsoleType::Switch2_FW19_JapanLocked:
    case ConsoleType::Switch2_FW20_International:
    case ConsoleType::Switch2_FW20_JapanLocked:
        home_to_date_time_Switch2_joycon_feedback(console, context, to_date_change);
        return;
    default:;
    }
    throw UserSetupError(
        console.logger(),
        "Unsupported console type: " + ConsoleType_strings(console_type)
    );
}



}
}
