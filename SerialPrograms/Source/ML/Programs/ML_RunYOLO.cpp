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
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
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



RunYOLO::RunYOLO()
{
    const std::string sam_model_path = RESOURCE_PATH() + "ML/yolov5_cpu.onnx";
    std::vector<std::string> labels = {"Bidoof"};
    if (std::filesystem::exists(sam_model_path)){
        m_yolo_session = std::make_unique<YOLOv5Session>(sam_model_path, labels);
    } else{
        std::cerr << "Error: no such YOLOv5 model path " << sam_model_path << "." << std::endl;
        QMessageBox box;
        box.critical(nullptr, "YOLOv5 Model Does Not Exist",
            QString::fromStdString("YOLOv5 model path" + sam_model_path + " does not exist."));
    }

}

void RunYOLO::program(NintendoSwitch::SingleSwitchProgramEnvironment& env, NintendoSwitch::ProControllerContext& context){
    if (!m_yolo_session){
        return;
    }

    VideoOverlaySet overlay_set(env.console.overlay());

    std::vector<YOLOv5Session::DetectionBox> output_boxes;
    while (true){
        VideoSnapshot last = env.console.video().snapshot();
        cv::Mat frame_mat_bgra = last.frame->to_opencv_Mat();
        cv::Mat frame_mat_rgb;
        cv::cvtColor(frame_mat_bgra, frame_mat_rgb, cv::COLOR_BGRA2RGB);
        
        output_boxes.clear();
        m_yolo_session->run(frame_mat_rgb, output_boxes);
        overlay_set.clear();
        for(const auto& box : output_boxes){
            overlay_set.add(COLOR_RED, box.box, m_yolo_session->label_name(box.label_idx));
        }
        // context.wait_until(last.timestamp + std::chrono::milliseconds(PERIOD_MILLISECONDS));
    }
}


}
}

