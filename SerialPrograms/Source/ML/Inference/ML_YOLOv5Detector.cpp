/*  YOLOv5 Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <filesystem>
#include <iostream>
#include <QMessageBox>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Globals.h"
#include "ML_YOLOv5Detector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ML{


YOLOv5Detector::~YOLOv5Detector() = default;

YOLOv5Detector::YOLOv5Detector()
{
    const std::string sam_model_path = RESOURCE_PATH() + "ML/yolov5.onnx";
    std::vector<std::string> labels = {"Bidoof"};
    if (std::filesystem::exists(sam_model_path)){
        m_yolo_session = std::make_unique<YOLOv5Session>(sam_model_path, std::move(labels));
    } else{
        std::cerr << "Error: no such YOLOv5 model path " << sam_model_path << "." << std::endl;
        QMessageBox box;
        box.critical(nullptr, "YOLOv5 Model Does Not Exist",
            QString::fromStdString("YOLOv5 model path" + sam_model_path + " does not exist."));
    }
}

bool YOLOv5Detector::detect(const ImageViewRGB32& screen){
    if (!m_yolo_session){
        return false;
    }

    cv::Mat frame_mat_bgra = screen.to_opencv_Mat();
    cv::Mat frame_mat_rgb;
    cv::cvtColor(frame_mat_bgra, frame_mat_rgb, cv::COLOR_BGRA2RGB);
    
    m_output_boxes.clear();
    m_yolo_session->run(frame_mat_rgb, m_output_boxes);

    return m_output_boxes.size() > 0;
}


YOLOv5Watcher::YOLOv5Watcher(VideoOverlay& overlay)
    : VisualInferenceCallback("YOLOv5")
    , m_overlay_set(overlay)
{
}

bool YOLOv5Watcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    if (!m_detector.session()){
        return false;
    }

    m_detector.detect(frame);

    m_overlay_set.clear();
    for(const auto& box : m_detector.detected_boxes()){
        m_overlay_set.add(COLOR_RED, box.box, m_detector.session()->label_name(box.label_idx));
    }
    return false;
}





}
}