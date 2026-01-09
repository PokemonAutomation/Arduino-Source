/*  ML Paddle OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  
 */

#include <iostream>
#include <fstream>
#include <numeric>
#include <limits>
#include "Common/Cpp/Exceptions.h"
#include "ML_PaddleOCRPipeline.h"

namespace PokemonAutomation{
namespace ML{

PaddleOCRPipeline::PaddleOCRPipeline(std::string det_path, std::string rec_path, std::string dict_path)
    : env(ORT_LOGGING_LEVEL_WARNING, "PaddleOCR")
        , det_session(env, std::wstring(det_path.begin(), det_path.end()).c_str(), Ort::SessionOptions{})
        , rec_session(env, std::wstring(rec_path.begin(), rec_path.end()).c_str(), Ort::SessionOptions{})
        , memory_info(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)) 
{
    LoadDictionary(dict_path);
}

void PaddleOCRPipeline::Run(const std::string& img_path) {
    #if 0
    cv::Mat img = cv::imread(img_path);
    if (img.empty()) return;

    // 1. Text Detection (simplified for brevity)
    // In practice, use DBPostProcess to get boxes from detection output
    std::vector<cv::Rect> boxes = {{10, 10, 100, 30}}; // Mock detected box

    for (auto& box : boxes) {
        cv::Mat cropped = img(box);
        std::string text = Recognize(cropped);
        std::cout << "Detected Text: " << text << std::endl;
    }
    #endif
}



void PaddleOCRPipeline::LoadDictionary(const std::string& path) {
    std::ifstream fs(path);
    std::string line;
    // m_dictionary.push_back("blank"); // CTC blank index
    while (std::getline(fs, line)){
        m_dictionary.push_back(line);
    }
}

std::string PaddleOCRPipeline::Recognize(const ImageViewRGB32& image, const ImageFloatBox& box) {

    cv::Mat cv_image = imageviewrgb32_to_cv_mat(image);
    cv::Rect cv_box = ImageFloatBox_to_cv_Rect(image.width(), image.height(), box);
    cv::Mat crop = cv_image(cv_box);
    cv::Mat crop_3ch;
    cv::cvtColor(crop, crop_3ch, cv::COLOR_BGRA2RGB);

    #if 0
    // Preprocess: Resize to height 48, maintain aspect ratio
    cv::Mat rec_input;
    cv::resize(crop, rec_input, cv::Size(320, 48)); // Fixed size for simplicity
    rec_input.convertTo(rec_input, CV_32FC3, 1.0 / 255.0);

    // Normalize and convert to NCHW
    std::vector<float> input_tensor_values = PreprocessNCHW(rec_input);
    // in 2026, the standard input shape for PaddleOCR recognition models (such as PP-OCRv4 or the latest PP-OCRv5 versions) is typically: {1, 3, 48, 320}
    std::vector<int64_t> shape = {1, 3, 48, 320};

    auto input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_values.data(), 
                                                        input_tensor_values.size(), shape.data(), shape.size());

                                                        
    #endif
    // If Dynamic: The shape is expressed as {1, 3, 48, -1}. In your C++ code, you would calculate the width based on the aspect ratio of the cropped box:
    // Formula: 
    // NewWidth = OriginalWidth * 48/OriginalHeight: 
    // Constraint: Many models require the width to be a multiple of 4 or 8.

   // dynamic width
        // 1. Calculate dynamic width (maintain aspect ratio)
    int target_h = 48;
    float aspect_ratio = (float)crop_3ch.cols / (float)crop_3ch.rows;
    int target_w = static_cast<int>(target_h * aspect_ratio);
    
    // target_w = std::max(32, (target_w / 8) * 8);  // no need to round to 32 for recognition, only for detection

    // 2. Preprocess: Resize and Normalize
    cv::Mat resized;
    cv::resize(crop_3ch, resized, cv::Size(target_w, target_h));
    resized.convertTo(resized, CV_32FC3, 2.0 / 255.0, -1.0);
    
    // 3. Convert HWC to NCHW
    std::vector<float> input_tensor_values = PreprocessNCHW(resized);

    // 4. Define Dynamic Shape
    std::vector<int64_t> input_shape = {1, 3, target_h, target_w};

    // 5. Create Tensor with the specific dynamic shape
    auto input_tensor = Ort::Value::CreateTensor<float>(
        memory_info, input_tensor_values.data(), input_tensor_values.size(), 
        input_shape.data(), input_shape.size()
    );

                      
    #if 0
    // Create the allocator
    Ort::AllocatorWithDefaultOptions allocator;    

    // Get Input Name
    Ort::AllocatedStringPtr input_name_ptr = rec_session.GetInputNameAllocated(0, allocator);
    const char* input_name = input_name_ptr.get();

    // Get Output Name
    Ort::AllocatedStringPtr output_name_ptr = rec_session.GetOutputNameAllocated(0, allocator);
    const char* output_name = output_name_ptr.get();
    #endif

    // const char* input_names[] = {"x"};
    // const char* output_names[] = {"softmax_0.tmp_0"}; // Check your model output name

    const char* input_names[] = {"x"};
    const char* output_names[] = {"fetch_name_0"};  // this is the output name as per Netron


    // auto outputs = rec_session.Run(Ort::RunOptions{nullptr}, input_name, &input_tensor, 1, output_name, 1);
    auto outputs = rec_session.Run(
        Ort::RunOptions{nullptr}, 
        input_names,   // char** 
        &input_tensor, // Ort::Value* (array of 1)
        1,             // input_count
        output_names,  // char**
        1              // output_count
    );
    std::cout << "Inference successful for width: " << target_w << std::endl;

    return DecodeCTC(outputs[0].GetTensorMutableData<float>(), outputs[0].GetTensorTypeAndShapeInfo().GetShape(), m_dictionary);
}


std::vector<float> PreprocessNCHW(cv::Mat& img) {
    std::vector<float> dst(img.rows * img.cols * 3);
    for (int c = 0; c < 3; ++c) {
        for (int i = 0; i < img.rows * img.cols; ++i) {
            dst[c * img.rows * img.cols + i] = ((float*)img.data)[i * 3 + c];
        }
    }
    return dst;
}

std::string DecodeCTC(float* data, const std::vector<int64_t>& shape, const std::vector<std::string>& dict) {
    std::string text = "";
    size_t seq_len = static_cast<size_t>(shape[1]);
    int64_t num_cls = shape[2];
    size_t last_index = 0; 
    for (size_t i = 0; i < seq_len; ++i) {
        float* row = data + i * num_cls;
        // 1. Get the character index with highest probability (Argmax)
        size_t argmax = std::distance(row, std::max_element(row, row + num_cls));

        // 2. CTC Decoding Rules:
        // Rule A: Index 0 is the CTC Blank. SKIP it.
        // Rule B: Skip consecutive duplicate characters (e.g., "aa" -> "a").
        if (argmax > 0 && argmax != last_index) {
            // Index 1 from the model maps to the 1st line of your .txt file (Vector index 0)
            size_t dict_idx = argmax - 1; 
            if (dict_idx < dict.size()) {
                text += dict[dict_idx];
            }
        }
        last_index = argmax;
    }
    return text;
}

#if 0
std::string DecodeCTC(const float* data, const std::vector<int64_t>& shape, const std::vector<std::string>& dict) {
    std::string text = "";
    int seq_len = static_cast<int>(shape[1]);
    int num_classes = static_cast<int>(shape[2]);
    int last_index = -1;

    for (int i = 0; i < seq_len; ++i) {
        // 1. Find the index with the maximum probability (Argmax)
        const float* row = data + (i * num_classes);
        int argmax_idx = std::distance(row, std::max_element(row, row + num_classes));

        // 2. CTC Logic: 
        // - Index 0 is the "blank" token in PaddleOCR
        // - Ignore consecutive duplicates (e.g., "aa" becomes "a")
        if (argmax_idx > 0 && argmax_idx != last_index) {
            if (argmax_idx < dict.size()) {
                text += dict[argmax_idx];
            }
        }
        last_index = argmax_idx;
    }
    return text;
}
#endif

template <typename _Tp>
_Tp safe_convert(size_t value) {
    if (value > static_cast<size_t>(std::numeric_limits<_Tp>::max())) {
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "safe_convert: Value too large for template type _Tp.");
    }
    return static_cast<_Tp>(value);
}

// Assumes QImage is in Format_RGB888 or Format_BGR888
cv::Mat imageviewrgb32_to_cv_mat(const ImageViewRGB32& image) {
    return cv::Mat(safe_convert<int>(image.height()), 
                   safe_convert<int>(image.width()), 
                   CV_8UC4, 
                   static_cast<void*>(const_cast<uint32_t*>(image.data())),
                   image.bytes_per_row());
}

cv::Rect ImageFloatBox_to_cv_Rect(size_t width, size_t height, const ImageFloatBox& box){
    ImagePixelBox pixelbox = floatbox_to_pixelbox(width, height, box);
    
    return cv::Rect(safe_convert<int>(pixelbox.min_x), 
                    safe_convert<int>(pixelbox.min_y), 
                    safe_convert<int>(pixelbox.width()), 
                    safe_convert<int>(pixelbox.height()));
}



}
}