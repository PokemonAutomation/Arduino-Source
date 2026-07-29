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
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Common/Cpp/Exceptions.h"
#include "ML/Models/ML_ONNXRuntimeHelpers.h"
#include "ML_PaddleOCRPipeline.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ML{


std::pair<std::string, std::string> PaddleOCRPipeline::get_paths(Language language){
    std::string base = RESOURCE_PATH() + "PaddleOCR/";
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
    : m_env{create_ORT_env()}
    // , det_session(env, std::wstring(det_path.begin(), det_path.end()).c_str(), Ort::SessionOptions{})
    , m_rec_session(create_session(m_env, rec_path, ML_MODEL_CACHE_PATH() + "PaddleOCRPipeline/", GlobalSettings::instance().USE_GPU_FOR_ML_INFERENCE))
    // , memory_info(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)) 
    , m_language(language)
    , m_input_name(m_rec_session.GetInputNameAllocated(0, Ort::AllocatorWithDefaultOptions{}).get())
    , m_output_name(m_rec_session.GetOutputNameAllocated(0, Ort::AllocatorWithDefaultOptions{}).get())
{
    load_dictionary(dict_path);
    
}

void PaddleOCRPipeline::run(const std::string& img_path){
    #if 0
    cv::Mat img = cv::imread(img_path);
    if (img.empty()) return;

    // 1. Text Detection (simplified for brevity)
    // In practice, use DBPostProcess to get boxes from detection output
    std::vector<cv::Rect> boxes = {{10, 10, 100, 30}}; // Mock detected box

    for (auto& box : boxes){
        cv::Mat cropped = img(box);
        std::string text = recognize(cropped);
        std::cout << "Detected Text: " << text << std::endl;
    }
    #endif
}



void PaddleOCRPipeline::load_dictionary(const std::string& path){
    std::ifstream fs(path);
    std::string line;
    // m_dictionary.push_back("blank"); // CTC blank index
    while (std::getline(fs, line)){
        m_dictionary.push_back(line);
    }
}

std::string PaddleOCRPipeline::recognize(const ImageViewRGB32& image){

    // 1. Convert Image to OpenCV image (cv::mat)
    cv::Mat cv_image_rgb = imageviewrgb32_to_cv_mat_rgb(image);
    if (cv_image_rgb.empty()) {
        return "";
    }

    
    // 2. Crop tightly around the text, with small safety margin
    cv::Mat cropped_image = crop_to_text_region(cv_image_rgb);
    if (cropped_image.empty()){
        return "";
    }

    // add horizontal padding to tall/narrow characters
    add_horizontal_padding(cropped_image);

    // 3. Calculate dynamic width (maintain aspect ratio)
    // the model shape is {1, 3, 48, dynamic_width}. Note that the height is fixed at 48 pixels
    // the input image must be scaled to match the height of 48, for the neural network
    int target_h = 48;
    float aspect_ratio = (float)cropped_image.cols / cropped_image.rows;

    int target_w = std::max(
        1,
        (int)std::round(target_h * aspect_ratio)
    );

    cv::Mat resized;
    cv::resize(
        cropped_image,
        resized,
        cv::Size(target_w, target_h),
        0,
        0,
        cv::INTER_LINEAR
    );


    // 4. Normalize
    // convert UC3 8-bit [0,255] to 32FC3 float [0,1], then use ImageNet Normalization
    // output = (Input * Scale) = (old_pixel * 1/255). This transforms [0,255] to range [0, 1]
    // TODO: determine if normalizing to [-1,1] is preferred or to perform ImageNet normalization (mean = [0.485, 0.456, 0.406] and std = [0.229, 0.224, 0.225])
    resized.convertTo(resized, CV_32FC3, 1.0 / 255.0);
    
    // 4b. Apply Mean/Std (Standard for PaddleOCR). except for Chinese
    // Mean: [0.485, 0.456, 0.406], Std: [0.229, 0.224, 0.225]
    if (!(m_language == Language::ChineseSimplified || 
        m_language == Language::ChineseTraditional ||
        m_language == Language::Japanese ||
        m_language == Language::Korean))
    {
        #if 0
        cv::Scalar mean(0.485, 0.456, 0.406);
        cv::Scalar std(0.229, 0.224, 0.225);
        cv::subtract(resized, mean, resized);
        cv::divide(resized, std, resized);
        #endif
    }
    
    
    // 5. Convert HWC to NCHW
    std::vector<float> input_tensor_values = preprocess_NCHW(resized);

    // 6. Define Dynamic Shape
    std::vector<int64_t> input_shape = {1, 3, target_h, target_w};

    // 7. Create tensor with its own managed memory
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

    try{
        // 8. Run the recognition session
        auto outputs = m_rec_session.Run(
            Ort::RunOptions{nullptr}, 
            input_names,   // char** 
            &input_tensor, // Ort::Value* (array of 1)
            1,             // input_count
            output_names,  // char**
            1              // output_count
        );
        return decode_CTC(outputs[0].GetTensorMutableData<float>(), outputs[0].GetTensorTypeAndShapeInfo().GetShape(), m_dictionary);
    }catch (Ort::Exception& e){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "PaddleOCRPipeline::recognize(): Failed." + std::string(e.what()));
    }
    
}

cv::Mat crop_to_text_region(const cv::Mat& image) {
    // first convert to grayscale
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_RGB2GRAY);

    // get a binary image, for cropping purposes
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255,
                cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    double ratio = cv::countNonZero(binary) /
                static_cast<double>(binary.total());

    // we want text pixels to be white for the next step
    // If most pixels are white, Otsu likely classified the background as foreground.
    // Flip it so text becomes white again.
    if (ratio > 0.5){
        cv::bitwise_not(binary, binary);
    }                

    // Find coordinates of all non-zero pixels (the text)
    std::vector<cv::Point> nonZeroCoords;
    cv::findNonZero(binary, nonZeroCoords);
    if (nonZeroCoords.empty()){
        return {};
    }

    // create bounding box for crop
    cv::Rect bbox = cv::boundingRect(nonZeroCoords);

    // increase bounding box slightly to add small safety margin
    int pad_x = std::max(4, bbox.width / 20);  // ~5%
    int pad_y = std::max(2, bbox.height / 20);  // ~5%

    bbox.x = std::max(0, bbox.x - pad_x);
    bbox.y = std::max(0, bbox.y - pad_y);

    bbox.width = std::min(
        image.cols - bbox.x,
        bbox.width + 2 * pad_x
    );

    bbox.height = std::min(
        image.rows - bbox.y,
        bbox.height + 2 * pad_y
    );

    // crop the original image based on the bounding box
    // the crop should be within bounds.
    cv::Mat cropped_image;
    cropped_image = image(bbox).clone();

    // static int i = 0;
    // i++;
    // cv::imwrite("aabinary" + std::to_string(i) + ".png", binary);
    // cv::imwrite("aacropped_image" + std::to_string(i) + ".png", cropped_image);

    return cropped_image;
}

void add_horizontal_padding(cv::Mat& image){
    if (image.empty()) {
        return;
    }

    int h = image.rows;
    int w = image.cols;
    constexpr float min_ratio = 0.5f;

    // add horizontal padding to tall/narrow characters
    if (h > 0 && (float)w / h < min_ratio) {
        int target_w = static_cast<int>(std::ceil(min_ratio * h));

        int left = (target_w - w) / 2;
        int right = target_w - w - left;

        cv::Scalar bg = estimate_background_color(image);
        cv::Mat padded_image;
        cv::copyMakeBorder(
            image,
            padded_image,
            0, 0,              // no vertical padding
            left, right,
            cv::BORDER_CONSTANT,
            bg
        );
        image = padded_image;
    }

    // static int i = 0;
    // i++;
    // cv::imwrite("aapadded" + std::to_string(i) + ".png", image);

}

cv::Scalar estimate_background_color(const cv::Mat& image) {
    if (image.empty() || image.type() != CV_8UC3) {
        return cv::Scalar(255, 255, 255);
    }

    // use the corner pixels to estimate the background color
    const cv::Vec3b* top = image.ptr<cv::Vec3b>(0);
    const cv::Vec3b* bottom = image.ptr<cv::Vec3b>(image.rows - 1);

    cv::Scalar tl(top[0]);
    cv::Scalar tr(top[image.cols - 1]);
    cv::Scalar bl(bottom[0]);
    cv::Scalar br(bottom[image.cols - 1]);

    return (tl + tr + bl + br) * 0.25;
}




std::vector<float> preprocess_NCHW(cv::Mat& img){
    std::vector<float> dst(img.rows * img.cols * 3);
    for (int c = 0; c < 3; ++c){
        for (int i = 0; i < img.rows * img.cols; ++i){
            dst[c * img.rows * img.cols + i] = ((float*)img.data)[i * 3 + c];
        }
    }
    return dst;
}

std::string decode_CTC(float* data, const std::vector<int64_t>& shape, const std::vector<std::string>& dict){
    std::string text = "";
    size_t seq_len = static_cast<size_t>(shape[1]);
    int64_t num_cls = shape[2];
    size_t last_index = 0; 
    for (size_t i = 0; i < seq_len; ++i){
        float* row = data + i * num_cls;
        // 1. Get the character index with highest probability (Argmax)
        size_t argmax = std::distance(row, std::max_element(row, row + num_cls));

        // 2. CTC Decoding Rules:
        // Rule A: Index 0 is the CTC Blank. Skip it.
        // Rule B: Skip consecutive duplicate characters (e.g., "aa" -> "a").
        if (argmax > 0 && argmax != last_index){
            // Index 1 from the model maps to the 1st line of your .txt file (Vector index 0)
            size_t dict_idx = argmax - 1; 
            if (dict_idx < dict.size()){
                text += dict[dict_idx];
            }
        }
        last_index = argmax;
    }
    return text;
}


template <typename _Tp>
_Tp safe_convert(size_t value){
    if (value > static_cast<size_t>(std::numeric_limits<_Tp>::max())){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "safe_convert: Value too large for template type _Tp.");
    }
    return static_cast<_Tp>(value);
}

// Convert ImageViewRGB32 (ARGB) to CV Mat (RGB). Create a new copy of the image.
cv::Mat imageviewrgb32_to_cv_mat_rgb(const ImageViewRGB32& image){
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