/*  ML ONNX Runtime Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Helper functions to work with ONNX Runtime library
 */

#include <iostream>
#include <onnxruntime_cxx_api.h>
#include "ML_ONNXRuntimeHelpers.h"

namespace PokemonAutomation{
namespace ML{

Ort::SessionOptions create_session_option(){
    return Ort::SessionOptions{};

    // create session using Apple ML

    // Ort::SessionOptions so;
    // std::unordered_map<std::string, std::string> provider_options;
    // provider_options["ModelFormat"] = "NeuralNetwork"; 
    // so.AppendExecutionProvider("CoreML", provider_options);
    // return so;
}


void print_model_input_output_info(const Ort::Session& session){
    const auto input_names = session.GetInputNames();
    const auto output_names = session.GetOutputNames();

    for (size_t i = 0; i < input_names.size(); ++i) {
        Ort::TypeInfo type_info = session.GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        std::vector<int64_t> input_dims = tensor_info.GetShape();

        std::cout << "Input " << i << ": " << input_names[i] << " Type " << tensor_info.GetElementType() <<  " Shape: [";
        for (size_t j = 0; j < input_dims.size(); ++j) {
            std::cout << input_dims[j];
            if (j < input_dims.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }

    for (size_t i = 0; i < output_names.size(); ++i) {
        Ort::TypeInfo type_info = session.GetOutputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        std::vector<int64_t> output_dims = tensor_info.GetShape();

        std::cout << "Output " << i << ": " << input_names[i] << " Type " << tensor_info.GetElementType() <<  " Shape: [";
        for (size_t j = 0; j < output_dims.size(); ++j) {
            std::cout << output_dims[j];
            if (j < output_dims.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
}

}
}