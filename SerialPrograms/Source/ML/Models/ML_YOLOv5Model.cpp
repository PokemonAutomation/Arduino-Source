/*  ML YOLOv5 Model
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Run YOLOv5 model for object detection
 */


#include <string>
#include <sstream>
#include <map>
//#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include "3rdParty/ONNX/OnnxToolsPA.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Logging/Logger.h"
#include "ML/Models/ML_ONNXRuntimeHelpers.h"
#include "ML_YOLOv5Model.h"

namespace PokemonAutomation{
namespace ML{


// Parse YOLO metadata "names" field to extract label names
// Expected format: "names: {0: 'label0', 1: 'label1', 2: 'label2', ...}"
std::vector<std::string> parse_yolo_metadata_names(const std::string& metadata_value){
    std::vector<std::string> label_names;
    std::map<int, std::string> label_map;

    // Find the opening brace
    size_t brace_start = metadata_value.find('{');
    size_t brace_end = metadata_value.rfind('}');

    if (brace_start == std::string::npos || brace_end == std::string::npos){
        return label_names;  // Return empty vector if format is invalid
    }

    std::string content = metadata_value.substr(brace_start + 1, brace_end - brace_start - 1);

    // Parse entries like "0: 'label0', 1: 'label1', ..."
    size_t pos = 0;
    while (pos < content.size()){
        // Find the index
        size_t colon_pos = content.find(':', pos);
        if (colon_pos == std::string::npos){
            break;
        }

        // Extract index
        std::string index_str = content.substr(pos, colon_pos - pos);
        // Trim whitespace
        index_str.erase(0, index_str.find_first_not_of(" \t\n\r"));
        index_str.erase(index_str.find_last_not_of(" \t\n\r") + 1);

        int index = std::stoi(index_str);

        // Find the label name (between quotes)
        size_t quote1 = content.find('\'', colon_pos);
        if (quote1 == std::string::npos){
            quote1 = content.find('"', colon_pos);  // Try double quotes
        }
        if (quote1 == std::string::npos){
            break;
        }

        size_t quote2 = content.find(content[quote1], quote1 + 1);  // Find matching quote
        if (quote2 == std::string::npos){
            break;
        }

        std::string label = content.substr(quote1 + 1, quote2 - quote1 - 1);
        label_map[index] = label;

        // Move to next entry
        pos = content.find(',', quote2);
        if (pos == std::string::npos){
            break;
        }
        pos++;  // Skip the comma
    }

    // Convert map to vector (in order)
    for (const auto& pair : label_map){
        if (pair.first != (int)label_names.size()){
            throw std::runtime_error("YOLO metadata labels are not sequential. Expected index " +
                std::to_string(label_names.size()) + " but found " + std::to_string(pair.first));
        }
        label_names.push_back(pair.second);
    }

    return label_names;
}


std::tuple<int, int, double, double> resize_image_with_border(
    const cv::Mat& input_image,
    cv::Mat& output_image,
    int target_width, int target_height,
    cv::Scalar border_color = cv::Scalar(0,0,0)
){
    int original_width = input_image.cols;
    int original_height = input_image.rows;

    double scale_x = static_cast<double>(target_width) / original_width;
    double scale_y = static_cast<double>(target_height) / original_height;
    double scale = std::min(scale_x, scale_y);

    int new_width = static_cast<int>(original_width * scale);
    int new_height = static_cast<int>(original_height * scale);
    new_width = std::min(new_width, target_width);
    new_height = std::min(new_height, target_height);

    if (new_width == 0 || new_height == 0){
        throw std::runtime_error("Input Image too small: " + std::to_string(original_width) + " x " + std::to_string(original_height));
    }

    cv::Mat resized_image;
    cv::resize(input_image, resized_image, cv::Size(new_width, new_height), 0, 0, cv::INTER_LINEAR); // INTER_AREA for shrinking

    int border_top = (target_height - new_height) / 2;
    int border_bottom = target_height - new_height - border_top;
    int border_left = (target_width - new_width) / 2;
    int border_right = target_width - new_width - border_left;

    cv::copyMakeBorder(resized_image, output_image, border_top, border_bottom, border_left, border_right, cv::BORDER_CONSTANT, border_color);

    return std::make_tuple(
        border_left, border_top,
        1.0 / new_width, 1.0 / new_height
    );
}


YOLOv5Session::YOLOv5Session(const std::string& model_path, bool use_gpu)
: m_env{create_ORT_env()}
, m_session_options(create_session_options(ML_MODEL_CACHE_PATH() + "YOLOv5", use_gpu))
, m_session{create_session(m_env, m_session_options, model_path, ML_MODEL_CACHE_PATH() + "YOLOv5")}
, m_memory_info{Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)}
, m_input_names{m_session.GetInputNames()}
, m_output_names{m_session.GetOutputNames()}
, m_model_input(3*YOLO5_INPUT_IMAGE_SIZE*YOLO5_INPUT_IMAGE_SIZE)
{
    // Extract YOLO labels from model metadata
    try {
        Ort::ModelMetadata metadata = m_session.GetModelMetadata();
        Ort::AllocatorWithDefaultOptions allocator;

        // Look for "names" key in custom metadata
        Ort::AllocatedStringPtr names_value = metadata.LookupCustomMetadataMapAllocated("names", allocator);

        if (names_value){
            std::string names_str(names_value.get());
            global_logger_tagged().log("YOLOv5: Found label metadata: " + names_str, COLOR_PURPLE);

            m_label_names = parse_yolo_metadata_names(names_str);

            if (!m_label_names.empty()){
                global_logger_tagged().log("YOLOv5: Extracted " + std::to_string(m_label_names.size()) +
                                            " labels from model metadata", COLOR_GREEN);
            }
        }else{
            throw std::runtime_error("YOLOv5 model does not have 'names' metadata to extract labels from");
        }
    } catch (const std::exception& e){
        throw std::runtime_error("YOLOv5: Failed to extract labels from metadata: " + std::string(e.what()));
    }

    if (m_session.GetOutputCount() != 1){
        throw std::runtime_error("YOLOv5 model does not have the correct output count, found count " + 
            std::to_string(m_session.GetOutputCount()));
    }

    std::vector<int64_t> output_dims = m_session.GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    if (output_dims.size() != 3 || output_dims[2] <= 5){
        throw std::runtime_error("YOLOv5 model does not have the correct output dimension, found shape " + to_string(output_dims));
    }
    m_output_shape[2] = output_dims[2];
    if (output_dims[2] - 5 != static_cast<int>(m_label_names.size())){
        throw std::runtime_error(
            "YOLOv5 model has " + std::to_string(output_dims[2]-5) +
            " output labels but YOLOv5Session was initialized with " + std::to_string(m_label_names.size()) + " labels"
        );
    }
    m_model_output.resize(YOLO5_NUM_CANDIDATES * m_output_shape[2]);
}

// input: rgb color order
void YOLOv5Session::run(const cv::Mat& input_image, std::vector<YOLOv5Session::DetectionBox>& output_boxes){
    CV_Assert(input_image.depth() == CV_8U);
    CV_Assert(input_image.channels() == 3);

    cv::Mat image_resized;

    int x_shift = 0, y_shift = 0;
    double x_scale = 1.0, y_scale = 1.0;
    std::tie(x_shift, y_shift, x_scale, y_scale) = resize_image_with_border(input_image, image_resized,
        YOLO5_INPUT_IMAGE_SIZE, YOLO5_INPUT_IMAGE_SIZE, cv::Scalar(114, 114, 114));

    // Declare a destination Mat for float32
    cv::Mat image_float;

    // Convert the uint8_image to image_float
    // The third argument (alpha) is a scaling factor.
    // For normalization to [0.0, 1.0], use 1.0 / 255.0.
    // For retaining original values (0-255), use 1.0.
    image_resized.convertTo(image_float, CV_32F, 1.0 / 255.0); 

    for (int c = 0, i = 0; c < 3; c++){
        for (int row = 0; row < image_float.rows; row++){
            for (int col = 0; col < image_float.cols; col++){
                float pixel_value = image_float.at<cv::Vec3f>(row, col)[c];
                m_model_input[i++] = pixel_value;
            }
        }
    }

    auto input_tensor = create_tensor<float>(m_memory_info, m_model_input, m_input_shape);
    auto output_tensor = create_tensor<float>(m_memory_info, m_model_output, m_output_shape);

    const char* input_name_c = m_input_names[0].data();
    const char* output_name_c = m_output_names[0].data();
    // auto start = std::chrono::steady_clock::now();
    m_session.Run(m_run_options, &input_name_c, &input_tensor, 1, &output_name_c, &output_tensor, 1);
    // auto end = std::chrono::steady_clock::now();
    // auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // std::cout << "Yolov5 inference time: " << milliseconds << " ms" << std::endl;

    const size_t cand_size = m_label_names.size() + 5;

    std::vector<cv::Rect> pixel_boxes;
    std::vector<int> indices;
    std::vector<float> scores;
    std::vector<size_t> labels;

    for(int i = 0; i < YOLO5_NUM_CANDIDATES; i++){
        float cx = m_model_output[cand_size*i];
        float cy = m_model_output[cand_size*i+1];
        float w = m_model_output[cand_size*i+2];
        float h = m_model_output[cand_size*i+3];
        float sc = m_model_output[cand_size*i+4];

        float max_score = 0.0;
        size_t pred_label = 0;  // predicted label
        for(size_t j_label = 0; j_label < m_label_names.size(); j_label++){
            float score = m_model_output[cand_size*i+5+j_label];
            if (score > max_score){
                max_score = score;
                pred_label = j_label;
            }
        }
        scores.push_back(max_score * sc); // sc is like a global confidence scale?
        pixel_boxes.emplace_back((int)(cx - w / 2 + 0.5), (int)(cy - h / 2 + 0.5), int(w + 0.5), int(h + 0.5));
        indices.push_back(i);
        labels.push_back(pred_label);
    }

    cv::dnn::NMSBoxes(pixel_boxes, scores, 0.2f, 0.45f, indices);

    // std::cout << "num found pixel_boxes " << indices.size() << std::endl;
    // return;

    for (int index : indices)
    {
        // Note the model predicts on (640x640) images, we need to convert the detected pixel_boxes back to
        // the full frame dimension.
        double x = (pixel_boxes[index].x - x_shift) * x_scale;
        double y = (pixel_boxes[index].y - y_shift) * y_scale;
        double w = pixel_boxes[index].width * x_scale;
        double h = pixel_boxes[index].height * y_scale;
        // std::cout << scores[index] << " " <<  x << " " << y << " " << w << " " << h << std::endl;

        YOLOv5Session::DetectionBox b;
        b.box = ImageFloatBox(x, y, w, h);
        b.score = scores[index];
        b.label_idx = labels[index];
        output_boxes.push_back(b);
    }
}


size_t YOLOv5Session::label_index(const std::string& label_name) const{
    for(size_t i = 0; i < m_label_names.size(); i++){
        if (label_name == m_label_names[i]){
            return i;
        }
    }
    return SIZE_MAX;
}

const YOLOv5Session::DetectionBox* find_detection(const std::vector<YOLOv5Session::DetectionBox>& detection, size_t label_idx){
    for(const auto& box : detection){
        if (box.label_idx == label_idx){
            return &box;
        }
    }
    return nullptr;
}


}
}
