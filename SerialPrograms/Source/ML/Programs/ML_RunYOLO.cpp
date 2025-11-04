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
        "Programs/NintendoSwitch/RunYOLO.html",
        "Run YOLO object detection model.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}



RunYOLO::RunYOLO()
    : MODEL_PATH(
        "<b>YOLO Model Path:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        RESOURCE_PATH() + "ML/yolov5.onnx",
        "*.onnx",
        "Path to YOLO .onnx model file"
    )
{
    PA_ADD_OPTION(MODEL_PATH);
}

void RunYOLO::program(NintendoSwitch::SingleSwitchProgramEnvironment& env, NintendoSwitch::ProControllerContext& context){
    std::string model_path = MODEL_PATH;
    YOLOv5Watcher watcher(env.console.overlay(), model_path);

    wait_until(env.console, context, WallClock::max(), {watcher});
}


}
}

