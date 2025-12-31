/*  YOLOv5 Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <filesystem>
#include <iostream>
#include <fstream>
#include <QMessageBox>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Globals.h"
#include "Common/Cpp/StringTools.h"
#include "Common/Cpp/PrettyPrint.h"
#include "ML_YOLOv5Detector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ML{


YOLOv5Detector::~YOLOv5Detector() = default;


YOLOv5Detector::YOLOv5Detector(const std::string& model_path)
    : m_model_path(model_path)
    , m_use_gpu(true)
{
    if (!model_path.ends_with(".onnx")){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, 
            "Error: YOLOv5 model path must end with .onnx. But got " + model_path + ".");
    }

    std::string label_file_path = model_path.substr(0, model_path.size() - 5) + "_label.txt";
    m_yolo_session = std::make_unique<YOLOv5Session>(m_model_path, m_use_gpu);
}

bool YOLOv5Detector::detect(const ImageViewRGB32& screen){
    if (!m_yolo_session){
        return false;
    }

    cv::Mat frame_mat_bgra = screen.to_opencv_Mat();
    cv::Mat frame_mat_rgb;
    cv::cvtColor(frame_mat_bgra, frame_mat_rgb, cv::COLOR_BGRA2RGB);

    // Run inference without holding the lock
    std::vector<YOLOv5Session::DetectionBox> temp_boxes;

    // fall back to CPU if fails with GPU.
    for(size_t i = 0; i < 2; i++){
        try{
            // if (m_use_gpu){ throw Ort::Exception("Testing.", ORT_FAIL); }  // to simulate GPU/CPU failure
            // If fails with GPU, fall back to CPU.
            m_yolo_session->run(frame_mat_rgb, temp_boxes);
            break;
        }catch(Ort::Exception& e){
            if (m_use_gpu){
                std::cerr << "Warning: YOLO session failed using the GPU. Will reattempt with the CPU.\n" << e.what() << std::endl;
                m_use_gpu = false;
                std::vector<std::string> labels = m_yolo_session->get_label_names();
                m_yolo_session = std::make_unique<YOLOv5Session>(m_model_path, m_use_gpu);
            }else{
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Error: YOLO session failed even when using the CPU." + std::string(e.what()));
            }
        }catch(...){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown error: YOLO session failed.");

        }

        if (i > 0){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Internal Program Error: This section of code shouldn't be reachable.");
        }
    }

    // Only lock when swapping results
    bool has_detections = temp_boxes.size() > 0;
    {
        WriteSpinLock lg(m_output_lock, "YOLOv5Detector::detect()");
        m_output_boxes = std::move(temp_boxes);
    }

    return has_detections;
}

std::vector<YOLOv5Session::DetectionBox> YOLOv5Detector::detected_boxes(){
    ReadSpinLock lg(m_output_lock, "YOLOv5Detector::detected_boxes()");
    return m_output_boxes;
}


YOLOv5Watcher::YOLOv5Watcher(VideoOverlay& overlay, const std::string& model_path)
    : VisualInferenceCallback("YOLOv5")
    , m_overlay_set(overlay)
    , m_detector(model_path)
{
}

bool YOLOv5Watcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    if (!m_detector.model_loaded()){
        return false;
    }

    m_detector.detect(frame);

    m_overlay_set.clear();
    for(const auto& box : m_detector.detected_boxes()){
        std::string text = m_detector.session()->label_name(box.label_idx) + ": " + tostr_fixed(box.score, 2);
        m_overlay_set.add(COLOR_RED, box.box, text);
    }
    return false;
}





}
}