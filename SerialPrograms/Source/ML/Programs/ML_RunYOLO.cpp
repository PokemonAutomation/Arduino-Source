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
#include "Common/Cpp/MemoryUtilization/MemoryUtilization.h"
#include "ML_RunYOLO.h"

namespace PokemonAutomation{
namespace ML{


RunYOLO_Descriptor::RunYOLO_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:RunYOLO",
        "Nintendo Switch", "Run YOLO",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/RunYOLO.md",
        "Run YOLO object detection model.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}



RunYOLO::RunYOLO() {}

void RunYOLO::program(NintendoSwitch::SingleSwitchProgramEnvironment& env, NintendoSwitch::ProControllerContext& context){

    MemoryUsage mu = process_memory_usage();
    std::cout << "Memory usage: " << mu.total_system_memory << " " << mu.total_used_system_memory << " " 
        << mu.process_physical_memory << " " << mu.process_virtual_memory << std::endl;
    
    YOLOv5Watcher watcher(env.console.overlay());

    wait_until(env.console, context, WallClock::max(), {watcher});
}


}
}

