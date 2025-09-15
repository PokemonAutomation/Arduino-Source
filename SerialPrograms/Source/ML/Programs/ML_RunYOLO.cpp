/*  ML Run YOLO Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Test YOLO detection model on Switch streams.
 */

#include <iostream>
#include <filesystem>
#include <QMessageBox>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Globals.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "ML/Inference/ML_YOLOv5Detector.h"
#include "ML_RunYOLO.h"

namespace PokemonAutomation{
namespace ML{


RunYOLO_Descriptor::RunYOLO_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:RunYOLO",
        "Nintendo Switch", "Run YOLO",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/RunYOLO.md",
        "Run YOLO object detection model.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}



RunYOLO::RunYOLO() {}

void RunYOLO::program(NintendoSwitch::SingleSwitchProgramEnvironment& env, NintendoSwitch::ProControllerContext& context){
    YOLOv5Watcher watcher(env.console.overlay());

    wait_until(env.console, context, WallClock::max(), {watcher});
}


}
}

