/*  ML Paddle OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  
 */

#ifndef PokemonAutomation_ML_PaddleOCRPipeline_H
#define PokemonAutomation_ML_PaddleOCRPipeline_H

#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include "Common/Cpp/Logging/TaggedLogger.h"
#include "Common/Cpp/Filesystem/FilePath.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{
namespace ML{


class PaddleOCRPipeline{
public:
    PaddleOCRPipeline(Language language);
    PaddleOCRPipeline(Language language, std::string rec_path, std::string dict_path);

    void run(const std::string& img_path);

    std::string recognize(const ImageViewRGB32& image);

    static std::pair<std::string, std::string> get_paths(Language language);

    std::string decode_CTC(float* data, const std::vector<int64_t>& shape, const std::vector<std::string>& dict);

private:
    void load_dictionary(const Filesystem::Path& path);

    // Ort::Session det_session;
    Ort::Session m_rec_session;
    // Ort::MemoryInfo memory_info;
    Language m_language;
    std::string m_input_name;
    std::string m_output_name;
    std::vector<std::string> m_dictionary;
    TaggedLogger m_logger;

};

// assumes the input image is RGB
cv::Mat crop_to_text_region_with_padding(const cv::Mat& image);

// if the image is narrow/tall, add horizontal padding
// modifies the input image
// assumes input image is RGB
void add_horizontal_padding(cv::Mat& image);

// assumes input image is RGB
cv::Scalar estimate_background_color(const cv::Mat& image);

// convert HWC (height, width, channels) to NCHW (batch N, channels C, height H, width W)
// HWC: pixels are interleaved. [B,G,R] [B,G,R] [B,G,R] ...
// NCHW: [All Blue Pixels...] [All Green Pixels...] [All Red Pixels...]
std::vector<float> preprocess_NCHW(cv::Mat& img);


cv::Mat imageviewrgb32_to_cv_mat_rgb(const ImageViewRGB32& image);

cv::Rect ImageFloatBox_to_cv_Rect(size_t width, size_t height, const ImageFloatBox& box);


}
}
#endif
