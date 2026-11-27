/*  ML YOLO Model
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Run a YOLO model for object detection (versions listed in YOLOVersion)
 */

#ifndef PokemonAutomation_ML_YOLOModel_H
#define PokemonAutomation_ML_YOLOModel_H


#include <opencv2/core/mat.hpp>
#include <onnxruntime_cxx_api.h>
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{
namespace ML{


// How to read a YOLO model's output. Set by the model's "yolo_version" metadata;
// a model without that key is read as YOLOv5.
enum class YOLOVersion{
    // "yolov5" or no key: [1, num_candidates, 5 + num_labels],
    // each row cx, cy, w, h, objectness, then one score per label.
    YOLOv5,
    // "yolo26-nms-free": Ultralytics YOLO26 exported with its NMS-free head (nms=False).
    // [1, max_det, 6], each row x1, y1, x2, y2, score, label index.
    YOLO26_NMS_FREE,
};

class YOLOSession{
public:
    struct DetectionBox{
        // Confidence value the model predicts on the detection. Range: [0.0, 1.0].
        // The higher the value, the more confident the model thinks the prediction is.
        double score;
        // Bounding box of the detected object.
        ImageFloatBox box;
        // Object label ID.
        size_t label_idx;
    };

    YOLOSession(const std::string& model_path, bool use_gpu);

    void run(const cv::Mat& input_image, std::vector<DetectionBox>& detections);

    const std::string& label_name(size_t idx) const { return m_label_names[idx]; }
    const std::vector<std::string>& get_label_names() const { return m_label_names; }
    // Return SIZE_MAX if no such label name exists.
    size_t label_index(const std::string& label_name) const;
    YOLOVersion version() const { return m_version; }

private:
    std::vector<std::string> m_label_names;
    YOLOVersion m_version = YOLOVersion::YOLOv5;

    Ort::Session m_session;
    Ort::MemoryInfo m_memory_info;
    Ort::RunOptions m_run_options;
    std::vector<std::string> m_input_names, m_output_names;

    // [1, 3, H, W] and the output shape (see YOLOVersion), read from the model in the constructor.
    std::array<int64_t, 4> m_input_shape{1, 3, 0, 0};
    std::array<int64_t, 3> m_output_shape{1, 0, 0};

    std::vector<float> m_model_input;
    std::vector<float> m_model_output;
};

// Find the first detection matching the given label ID from a YOLOSession detection output.
const YOLOSession::DetectionBox* find_detection(const std::vector<YOLOSession::DetectionBox>& detection, size_t label_idx);


}
}
#endif
