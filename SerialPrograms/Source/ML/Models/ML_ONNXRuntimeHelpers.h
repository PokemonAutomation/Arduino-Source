/*  ML ONNX Runtime Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Helper functions to work with ONNX Runtime library
 */

#ifndef PokemonAutomation_ML_ONNXRuntimeHelpers_H
#define PokemonAutomation_ML_ONNXRuntimeHelpers_H


#include <vector>
#include <sstream>
#include <onnxruntime_cxx_api.h>

namespace PokemonAutomation{
namespace ML{


// Create an ONNX Runtiem session options object.
// For now it only sets the session to be CPU. In future we can create options for GPU or macOS MPS
Ort::SessionOptions create_session_option();

// Handy function to create an ONNX Runtime tensor view class from a vector-like `buffer` object holding
// the tensor data and an array-like `shape` object that represents the dimension of the tensor.
template<typename T, class Buffer, class Shape>
Ort::Value create_tensor(const OrtMemoryInfo* memory_info, Buffer& buffer, const Shape& shape){
    return Ort::Value::CreateTensor<T>(memory_info, buffer.data(), buffer.size(),
        shape.data(), shape.size());
}


//  Print vector as std::string. Useful for printing debugging info on tensor shapes
template<typename T>
std::string to_string(std::vector<T>& vec){
    std::ostringstream os;
    os << "[";
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(os, ", "));
    os << "]";
    return os.str();
}

//  Print model input and output types and shapes to cout. Useful for debugging.
void print_model_input_output_info(const Ort::Session& session);


}
}
#endif