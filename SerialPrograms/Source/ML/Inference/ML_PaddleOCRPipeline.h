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



namespace PokemonAutomation{
namespace ML{


class PaddleOCRPipeline {
public:
    PaddleOCRPipeline(std::string det_path, std::string rec_path, std::string dict_path);

    void Run(const std::string& img_path);

    std::string Recognize(cv::Mat& crop);

private:
    void LoadDictionary(const std::string& path);

    Ort::Env env;
    Ort::Session det_session, rec_session;
    Ort::MemoryInfo memory_info;
    std::vector<std::string> m_dictionary;    

};

std::vector<float> PreprocessNCHW(cv::Mat& img);

std::string DecodeCTC(float* data, const std::vector<int64_t>& shape, const std::vector<std::string>& dict);


}
}
#endif