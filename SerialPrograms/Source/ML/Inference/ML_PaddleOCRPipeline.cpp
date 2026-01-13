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
#include "CommonFramework/Globals.h"
#include "Common/Cpp/Exceptions.h"
#include "ML_PaddleOCRPipeline.h"

namespace PokemonAutomation{
namespace ML{


static std::pair<std::string, std::string> get_paths(Language language){
    std::string base = RESOURCE_PATH() + "ML/";
    switch(language){
    case Language::None:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to call OCR without a language.");
    case Language::English:
        return {base + "english/rec.onnx", base + "english/dict.txt"};
    case Language::Japanese:
        return {base + "chinese/rec.onnx", base + "chinese/dict.txt"};
    case Language::Spanish:
        return {base + "latin/rec.onnx", base + "latin/dict.txt"};
    case Language::French:
        return {base + "latin/rec.onnx", base + "latin/dict.txt"};
    case Language::German:
        return {base + "latin/rec.onnx", base + "latin/dict.txt"};   
    case Language::Italian:
        return {base + "latin/rec.onnx", base + "latin/dict.txt"};   
    case Language::Korean:
        return {base + "korean/rec.onnx", base + "korean/dict.txt"};  
    case Language::ChineseSimplified:
        return {base + "chinese/rec.onnx", base + "chinese/dict.txt"};
    case Language::ChineseTraditional:
        return {base + "chinese/rec.onnx", base + "chinese/dict.txt"}; 
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to call OCR on an unknown language.");
    }
}

PaddleOCRPipeline::PaddleOCRPipeline(Language language)
    : PaddleOCRPipeline(language, get_paths(language).first, get_paths(language).second)
{}

PaddleOCRPipeline::PaddleOCRPipeline(Language language, std::string rec_path, std::string dict_path)
    : env(ORT_LOGGING_LEVEL_WARNING, "PaddleOCR")
        // , det_session(env, std::wstring(det_path.begin(), det_path.end()).c_str(), Ort::SessionOptions{})
        , rec_session(env, std::wstring(rec_path.begin(), rec_path.end()).c_str(), Ort::SessionOptions{})
        , memory_info(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)) 
        , m_language(language)
        , m_input_name(rec_session.GetInputNameAllocated(0, Ort::AllocatorWithDefaultOptions{}).get())
        , m_output_name(rec_session.GetOutputNameAllocated(0, Ort::AllocatorWithDefaultOptions{}).get())
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

std::string PaddleOCRPipeline::Recognize(const ImageViewRGB32& image) {

    // 1. Convert Image to OpenCV image (cv::mat)
    cv::Mat cv_image_rgb = imageviewrgb32_to_cv_mat_rgb(image);
    
    // 1b. Whitespace Trimming Logic
    cv::Mat gray;
    cv::cvtColor(cv_image_rgb, gray, cv::COLOR_BGR2GRAY);

    // Invert image for findNonZero: text becomes white (255), background black (0)
    cv::Mat binary_inv;
    cv::threshold(gray, binary_inv, 254, 255, cv::THRESH_BINARY_INV);

    // Find coordinates of all text pixels
    std::vector<cv::Point> nonZeroCoords;
    cv::findNonZero(binary_inv, nonZeroCoords);

    cv::Mat cropped_image;
    if (!nonZeroCoords.empty()) {
        // Get the minimal bounding rectangle for the text
        cv::Rect boundingBox = cv::boundingRect(nonZeroCoords);
        
        // Add a small buffer/padding around the box if needed (optional)
        boundingBox.x = std::max(0, boundingBox.x - 2);
        boundingBox.y = std::max(0, boundingBox.y - 2);
        boundingBox.width = std::min(cv_image_rgb.cols - boundingBox.x, boundingBox.width + 4);
        boundingBox.height = std::min(cv_image_rgb.rows - boundingBox.y, boundingBox.height + 4);

        // Crop the original color image
        cropped_image = cv_image_rgb(boundingBox);
        // static int i = 0;
        // cv::imwrite("output" + std::to_string(i) + ".png", cropped_image);
        // i++;
    } else {
        return ""; // Return empty string if no text is detected in the region
    }
    
    // 2a. Calculate dynamic width (maintain aspect ratio)
    // the model shape is {1, 3, 48, dynamic_width}. Note that the height is fixed at 48 pixels
    // the input image must be scaled to match the height of 48, for the neural network
    int target_h = 48;
    float aspect_ratio = (float)cropped_image.cols / (float)cropped_image.rows;
    int target_w = static_cast<int>(target_h * aspect_ratio);

    if (target_w <= 0) return "";
    
    // 2b. Resize
    cv::Mat resized;
    cv::resize(cropped_image, resized, cv::Size(target_w, target_h));

    // 3. Normalize
    // convert UC3 8-bit [0,255] to 32FC3 float [0,1], then use ImageNet Normalization
    // output = (Input * Scale) = (old_pixel * 1/255). This transforms [0,255] to range [0, 1]
    // TODO: determine if normalizing to [-1,1] is preferred or to perform ImageNet normalization (mean = [0.485, 0.456, 0.406] and std = [0.229, 0.224, 0.225])
    resized.convertTo(resized, CV_32FC3, 1.0 / 255.0);
    #if 0
    // 3b. Apply Mean/Std (Standard for PaddleOCR). except for Chinese
    // Mean: [0.485, 0.456, 0.406], Std: [0.229, 0.224, 0.225]
    if (!(m_language == Language::ChineseSimplified || 
        m_language == Language::ChineseTraditional ||
        m_language == Language::Japanese ||
        m_language == Language::Korean))
    {
        cv::Scalar mean(0.485, 0.456, 0.406);
        cv::Scalar std(0.229, 0.224, 0.225);
        cv::subtract(resized, mean, resized);
        cv::divide(resized, std, resized);
    }
    #endif
    
    // 3. Convert HWC to NCHW
    std::vector<float> input_tensor_values = PreprocessNCHW(resized);

    // 4. Define Dynamic Shape
    std::vector<int64_t> input_shape = {1, 3, target_h, target_w};

    // 5. Create tensor with its own managed memory
    Ort::AllocatorWithDefaultOptions allocator;    
    auto input_tensor = Ort::Value::CreateTensor<float>(
        allocator, input_shape.data(), input_shape.size()
    );

    // Copy your processed data into that memory
    std::memcpy(input_tensor.GetTensorMutableData<float>(), 
                input_tensor_values.data(), 
                input_tensor_values.size() * sizeof(float));

    const char* input_names[] = {m_input_name.c_str()};
    const char* output_names[] = {m_output_name.c_str()};  

    try {
        // 7. Run the recognition session
        auto outputs = rec_session.Run(
            Ort::RunOptions{nullptr}, 
            input_names,   // char** 
            &input_tensor, // Ort::Value* (array of 1)
            1,             // input_count
            output_names,  // char**
            1              // output_count
        );
        return DecodeCTC(outputs[0].GetTensorMutableData<float>(), outputs[0].GetTensorTypeAndShapeInfo().GetShape(), m_dictionary);
    }catch(Ort::Exception& e){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "PaddleOCRPipeline::Recognize(): Failed." + std::string(e.what()));
    }
    
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
        // Rule A: Index 0 is the CTC Blank. Skip it.
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


template <typename _Tp>
_Tp safe_convert(size_t value) {
    if (value > static_cast<size_t>(std::numeric_limits<_Tp>::max())) {
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "safe_convert: Value too large for template type _Tp.");
    }
    return static_cast<_Tp>(value);
}

// Convert ImageViewRGB32 (ARGB) to CV Mat (RGB). Create a new copy of the image.
cv::Mat imageviewrgb32_to_cv_mat_rgb(const ImageViewRGB32& image) {
    // 1. Wrap the existing 4-channel data without copying memory
    cv::Mat bgra_wrap = image.to_opencv_Mat();

    // 2. Convert and copy to a new 3-channel RGB Mat
    cv::Mat rgb;
    cv::cvtColor(bgra_wrap, rgb, cv::COLOR_BGRA2RGB);

    return rgb;
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