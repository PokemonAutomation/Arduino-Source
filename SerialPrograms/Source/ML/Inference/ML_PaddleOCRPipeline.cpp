/*  ML Paddle OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  
 */

#include <fstream>
#include <limits>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "CommonFramework/ImageTypes/ImageRGB32_OpenCV.h"
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
    // , det_session(env, std::wstring(det_path.begin(), det_path.end()).c_str(), Ort::SessionOptions{})
    : m_rec_session(
        create_session(
            rec_path,
            ML_MODEL_CACHE_PATH() + "PaddleOCRPipeline/",
            PerformanceOptions::instance().ONNX_OPTIONS.USE_GPU
        )
    )
    // , memory_info(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)) 
    , m_language(language)
    , m_input_name(m_rec_session.GetInputNameAllocated(0, Ort::AllocatorWithDefaultOptions{}).get())
    , m_output_name(m_rec_session.GetOutputNameAllocated(0, Ort::AllocatorWithDefaultOptions{}).get())
    , m_logger(global_logger_raw(), "OCR")
{
    load_dictionary(Filesystem::Path(dict_path));
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



void PaddleOCRPipeline::load_dictionary(const Filesystem::Path& path){

    const bool debugging = STATIC_GLOBALS.PADDLE_OCR_DEBUG;

    if (debugging){
        m_logger.log("[OCR-INFO] Loading dictionary from: " + path.string());
        m_logger.log("[OCR-INFO] Current working directory: " +
                    Filesystem::current_path().string());

        std::error_code ec;
        const auto absolute_path = Filesystem::absolute(path);

        if (!ec) {
            m_logger.log("[OCR-INFO] Absolute dictionary path: " +
                        absolute_path.string());

            m_logger.log("[OCR-INFO] Dictionary exists: " +
                        std::string(Filesystem::exists(absolute_path) ? "true" : "false"));

            if (Filesystem::exists(absolute_path)) {
                const auto file_size = Filesystem::file_size(absolute_path, ec);

                if (!ec) {
                    m_logger.log("[OCR-INFO] Dictionary file size: " +
                                std::to_string(file_size) + " bytes");
                }
            }
        }
    }

    std::ifstream fs_file(path.stdpath());

    if (!fs_file.is_open()) {
        m_logger.log("[OCR-ERROR] Failed to open dictionary: " + path.string());
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "PaddleOCRPipeline::load_dictionary(): Failed to open dictionary.", path.string());
    }

    std::string line;
    while (std::getline(fs_file, line)) {
        m_dictionary.push_back(line);
    }

    if (fs_file.bad()) {
        m_logger.log("[OCR-ERROR] I/O error while reading dictionary: " + path.string());
    }

    if (debugging){
        m_logger.log("[OCR-INFO] Loaded " +
                    std::to_string(m_dictionary.size()) +
                    " dictionary entries");
    }
}


std::string PaddleOCRPipeline::recognize(const ImageViewRGB32& image){

    const bool debugging = STATIC_GLOBALS.PADDLE_OCR_DEBUG;

    // 1. Convert Image to OpenCV image (cv::mat)
    cv::Mat cv_image_rgb = imageviewrgb32_to_cv_mat_rgb(image);
    if (cv_image_rgb.empty()) {
        m_logger.log("[OCR-DEBUG] Input was an empty image.");
        return "";
    }

    
    // 2. Crop tightly around the text, with small safety margin
    cv::Mat cropped_image = crop_to_text_region_with_padding(cv_image_rgb);
    if (cropped_image.empty()){
        m_logger.log("[OCR-DEBUG] Crop to text region returned empty image.");
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

    if (target_w <= 0 || target_w > 8192){
        m_logger.log("[OCR-ERROR] Abnormally scaled target width calculated: " + std::to_string(target_w));
        return "";
    }

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
    switch (m_language){
    case Language::ChineseSimplified:
    case Language::ChineseTraditional:
    case Language::Japanese:
    case Language::Korean:
        break;
    default:;
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


    size_t expected_elements = 1 * 3 * target_h * target_w;
    if (debugging){
        m_logger.log("[OCR-DEBUG] Cropped image constraints - Width: " + std::to_string(cropped_image.cols) 
            + ", Height: " + std::to_string(cropped_image.rows) 
            + ", Channels: " + std::to_string(cropped_image.channels()) 
            + ", Total Pixels: " + std::to_string(cropped_image.total()));

        size_t nan_count = 0;
        size_t subnormal_count = 0;
        for (float val : input_tensor_values) {
            if (std::isnan(val)) {
                nan_count++;
            } else if (val != 0.0f && std::fpclassify(val) == FP_SUBNORMAL) {
                subnormal_count++;
            }
        }
        m_logger.log("[OCR-DEBUG] Tensor payload validation - Total Floats: " + std::to_string(input_tensor_values.size())
                + ", NaNs detected: " + std::to_string(nan_count) 
                + ", Subnormal (denormal) values: " + std::to_string(subnormal_count));
    
        // Validate expected payload sizing matches matrix dimensionality
        m_logger.log("[OCR-DEBUG] Shape Definition - NCHW: [" + std::to_string(input_shape[0]) + "," + std::to_string(input_shape[1]) 
                + "," + std::to_string(input_shape[2]) + "," + std::to_string(input_shape[3]) + "]. Expected Elements: " + std::to_string(expected_elements));

    }

    if (input_tensor_values.size() != static_cast<size_t>(expected_elements)) {
        m_logger.log("[OCR-ERROR] Vector length vs input_shape calculation mismatch!");
        m_logger.log("[OCR-ERROR] Fatal memory stride mismatch. Vector size (" + std::to_string(input_tensor_values.size())
                    + ") does not match shape requirement (" + std::to_string(expected_elements));
        return "";
    }

    // 7. Create tensor with its own managed memory
    Ort::AllocatorWithDefaultOptions allocator;    
    auto input_tensor = Ort::Value::CreateTensor<float>(
        allocator,
        input_shape.data(),
        input_shape.size()
    );

    // Copy your processed data into that memory
    std::memcpy(
        input_tensor.GetTensorMutableData<float>(),
        input_tensor_values.data(),
        input_tensor_values.size() * sizeof(float)
    );

    const char* input_names[] = {m_input_name.c_str()};
    const char* output_names[] = {m_output_name.c_str()};  

    try{
        if (debugging) {
            m_logger.log("[OCR-DEBUG] Calling m_rec_session.Run() now...");
        }

        // 8. Run the recognition session
        auto outputs = m_rec_session.Run(
            Ort::RunOptions{nullptr}, 
            input_names,   // char** 
            &input_tensor, // Ort::Value* (array of 1)
            1,             // input_count
            output_names,  // char**
            1              // output_count
        );
        return decode_CTC(
            outputs[0].GetTensorMutableData<float>(),
            outputs[0].GetTensorTypeAndShapeInfo().GetShape(),
            m_dictionary
        );
    }catch (Ort::Exception& e){
        throw InternalProgramError(
            nullptr,
            PA_CURRENT_FUNCTION,
            "PaddleOCRPipeline::recognize(): Failed." + std::string(e.what())
        );
    }
    
}

cv::Mat crop_to_text_region_with_padding(const cv::Mat& image) {
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

    // get the current gap between the text and the edge
    int top_gap = bbox.y;
    int bottom_gap = image.rows - (bbox.y + bbox.height);
    int left_gap = bbox.x;
    int right_gap = image.cols - (bbox.x + bbox.width);

    // calculate the desired padding
    int pad_x = std::max(4, bbox.width / 10);  // ~10%
    int pad_y = std::max(2, bbox.height / 10);  // ~10%


    // static int i = 0;
    // i++;
    cv::Mat cropped_image;
    if (top_gap >= pad_y && bottom_gap >= pad_y && left_gap >= pad_x && right_gap >= pad_x){
        // Original image has plenty of padding.
        // We just expand the bounding box out by pad_x, pad_y and crop directly.

        // we guarantee that bbox.x - pad_x >= 0
        // and image.cols >= bbox.width + (pad_x * 2)
        cv::Rect optimal_crop(
            bbox.x - pad_x,
            bbox.y - pad_y,
            bbox.width + (pad_x * 2),
            bbox.height + (pad_y * 2)
        );
        // crop the original image
        // the crop should be within bounds.
        cropped_image = image(optimal_crop).clone();
    }else{

        // add more padding to original image
        cv::Scalar bg = estimate_background_color(image);
        cv::Mat padded_image;
        cv::copyMakeBorder(
            image,
            padded_image,
            pad_y, pad_y,
            pad_x, pad_x,
            cv::BORDER_CONSTANT,
            bg
        );

        // cv::imwrite(std::to_string(i) + "-padded" + ".png", padded_image);

        cv::Rect final_crop(
            bbox.x, // (bbox.x + pad_x) - pad_x cancels out perfectly to just bbox.x
            bbox.y, // (bbox.y + pad_y) - pad_y cancels out perfectly to just bbox.y
            bbox.width + (pad_x * 2),
            bbox.height + (pad_y * 2)
        );

        // crop the padded image
        // the crop should be within bounds.
        
        cropped_image = padded_image(final_crop).clone();
    }


    // cv::imwrite(std::to_string(i) + "-binary" + ".png", binary);
    // cv::imwrite(std::to_string(i) + "-cropped_image" +".png", cropped_image);

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
    // cv::imwrite(std::to_string(i) + "-horiz-padded" + ".png", image);

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
    const int rows = img.rows;
    const int cols = img.cols;
    const int channels = 3;
    
    // Allocate a flat memory buffer big enough for all channels
    std::vector<float> dst(rows * cols * channels);

    // Define the size of one complete "color plane" (channel)
    const int plane_size = rows * cols;

    // Loop through the image row-by-row
    for (int y = 0; y < rows; ++y) {
        // Safely locate the exact memory address for the start of row 'y'
        const float* row_ptr = img.ptr<float>(y);
        
        // Loop through every pixel column in the current row
        for (int x = 0; x < cols; ++x) {
            // Calculate the 1D coordinate of the pixel inside a flat 2D plane
            int linear_idx = y * cols + x;
            
            // Extract the interleaved BGR channels explicitly
            dst[0 * plane_size + linear_idx] = row_ptr[x * channels + 0]; // Channel 0
            dst[1 * plane_size + linear_idx] = row_ptr[x * channels + 1]; // Channel 1
            dst[2 * plane_size + linear_idx] = row_ptr[x * channels + 2]; // Channel 2
        }
    }
    return dst;
}



std::string PaddleOCRPipeline::decode_CTC(float* data, const std::vector<int64_t>& shape, const std::vector<std::string>& dict){

    const bool debugging = STATIC_GLOBALS.PADDLE_OCR_DEBUG;

    std::string text = "";
    size_t seq_len = static_cast<size_t>(shape[1]);
    int64_t num_cls = shape[2];
    size_t last_index = 0; 

    // Initial boundary logging configuration
    if (debugging){
        m_logger.log("[OCR-CTC-DEBUG] Starting decode_CTC. Sequence Length: " + std::to_string(seq_len) + 
                ", Total Classes: " + std::to_string(num_cls) + 
                ", Dictionary Size: " + std::to_string(dict.size()));
    }

    if (dict.empty()) {
        m_logger.log("[OCR-CTC-ERROR] FATAL: Dictionary payload array is empty! Parsing loops will fail to resolve text indicators.");
    }

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
                if (debugging) {
                    m_logger.log("[OCR-CTC-DEBUG] Step " + std::to_string(i) + 
                               ": Predicted Argmax = " + std::to_string(argmax) + 
                               " -> Target Dict Index = " + std::to_string(dict_idx) +
                               " (Character resolved: '" + dict[dict_idx] + "')");
                }
                text += dict[dict_idx];
            }else {
                m_logger.log("[OCR-CTC-ERROR] Step " + std::to_string(i) + 
                            ": Predicted Argmax = " + std::to_string(argmax) + 
                            " -> Target Dict Index = " + std::to_string(dict_idx) + 
                            " (ERROR: Calculated index is out of bounds for the dictionary memory layout!)");
            }
        }
        last_index = argmax;
    }

    if (debugging) {
        m_logger.log("[OCR-CTC-DEBUG] Complete loop execution tracking finish. Resulting string extraction: '" + text + "'");
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
    cv::Mat bgra_wrap = to_OpenCV_ref(image);

    // 2. Convert and copy to a new 3-channel RGB Mat
    cv::Mat rgb;
    cv::cvtColor(bgra_wrap, rgb, cv::COLOR_BGRA2RGB);

    return rgb;
}

cv::Rect ImageFloatBox_to_cv_Rect(size_t width, size_t height, const ImageFloatBox& box){
    ImagePixelBox pixelbox = floatbox_to_pixelbox(width, height, box);
    
    return cv::Rect(
        safe_convert<int>(pixelbox.min_x),
        safe_convert<int>(pixelbox.min_y),
        safe_convert<int>(pixelbox.width()),
        safe_convert<int>(pixelbox.height())
    );
}



}
}
