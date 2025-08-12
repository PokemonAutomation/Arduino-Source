/*  ML YOLOv5 Model
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Run YOLOv5 model for object detection
 */

#ifndef PokemonAutomation_ML_YOLOv5Model_H
#define PokemonAutomation_ML_YOLOv5Model_H


#include <onnxruntime_cxx_api.h>
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{
namespace ML{


class YOLOv5Session{
public:
    struct DetectionBox{
        double score;
        ImageFloatBox box;
        size_t label_idx;
    };

    YOLOv5Session(const std::string& model_path, std::vector<std::string> label_names);

    void run(const cv::Mat& input_image, std::vector<DetectionBox>& detections);

    const std::string& label_name(size_t idx) const { return m_label_names[idx]; }
    
private:
    const int YOLO5_INPUT_IMAGE_SIZE = 640;
    const int YOLO5_NUM_CANDIDATES = 25200;

    std::vector<std::string> m_label_names;

    Ort::Env m_env;
    Ort::SessionOptions m_session_options;
    Ort::Session m_session;
    Ort::MemoryInfo m_memory_info;
    Ort::RunOptions m_run_options;
    std::vector<std::string> m_input_names, m_output_names;

    const std::array<int64_t, 4> m_input_shape{1, 3, YOLO5_INPUT_IMAGE_SIZE, YOLO5_INPUT_IMAGE_SIZE};
    std::array<int64_t, 3> m_output_shape{1, YOLO5_NUM_CANDIDATES, 0};

    std::vector<float> m_model_input;
    std::vector<float> m_model_output;
};


}
}
#endif
