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
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"



namespace PokemonAutomation{
namespace ML{


class PaddleOCRPipeline {
public:
    PaddleOCRPipeline(Language language);
    PaddleOCRPipeline(std::string rec_path, std::string dict_path);

    void Run(const std::string& img_path);

    std::string Recognize(const ImageViewRGB32& image);

private:
    void LoadDictionary(const std::string& path);

    Ort::Env env;
    // Ort::Session det_session;
    Ort::Session rec_session;
    Ort::MemoryInfo memory_info;
    std::vector<std::string> m_dictionary;    

};

// convert HCW (height, width, channels) to NCHW (batch N, channels C, height H, width W)
std::vector<float> PreprocessNCHW(cv::Mat& img);

std::string DecodeCTC(float* data, const std::vector<int64_t>& shape, const std::vector<std::string>& dict);

cv::Mat imageviewrgb32_to_cv_mat_rgb(const ImageViewRGB32& image);

cv::Rect ImageFloatBox_to_cv_Rect(size_t width, size_t height, const ImageFloatBox& box);


}
}
#endif