/*  ML ONNX Runtime Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Helper functions to work with ONNX Runtime library
 */

#include <QString>
#include <QFile>
#include <QCryptographicHash>
#include <QByteArray>

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <onnxruntime_cxx_api.h>
#include "3rdParty/ONNX/OnnxToolsPA.h"
#include "Common/Compiler.h"
#include "ML_ONNXRuntimeHelpers.h"

namespace fs = std::filesystem;

namespace PokemonAutomation{
namespace ML{


// Computes the cryptographic hash of a file.
std::string create_file_hash(const std::string& filepath){
    QFile file(QString::fromStdString(filepath));
    if (!file.open(QIODevice::ReadOnly)) {
        return "";
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (hash.addData(&file)) {
        return hash.result().toHex(0).toStdString();
    } else {
        return "";
    }
}


Ort::SessionOptions create_session_options(const std::string& model_cache_path){
    Ort::SessionOptions so;
    std::cout << "Set potential model cache path in session options: " << model_cache_path << std::endl;
#if __APPLE__
    // create session using Apple ML acceleration library CoreML
    std::unordered_map<std::string, std::string> provider_options;
    // See for provider options: https://onnxruntime.ai/docs/execution-providers/CoreML-ExecutionProvider.html
    // "NeuralNetwork" is a faster ModelFormat than "MLProgram".
    provider_options["ModelFormat"] = std::string("NeuralNetwork");
    provider_options["ModelCacheDirectory"] = model_cache_path;

    // provider_options["MLComputeUnits"] = "ALL";
    // provider_options["RequireStaticInputShapes"] = "0";
    // provider_options["EnableOnSubgraphs"] = "0";
    so.AppendExecutionProvider("CoreML", provider_options);
#endif

    // use CPU session
    return so;
}

// Check the model file cache integrity by checking the existence of a flag file and the model hash stored 
// in the flag file. If the flag does not exist, we assume the file cache does not exist or is broken.
// If the hash stored in the flag file does not match the model file, the model file is a new model, delete
// the old cache.
// Return
// - bool: whether to write flag file after cache is created
// - string: the model file hash to write into the flag file after Ort::Session is built and the cache is created.
//
// model_cache_path: Folder path to store model cache. This name is better to be unique for each model for
//   easier file management.
// model_path: the model path to load the ML model. This is needed to ensure we delete the old model cache
//   when a new model
std::pair<bool, std::string> clean_up_old_model_cache(const std::string& model_cache_path, const std::string& model_path){
    std::string file_hash = create_file_hash(model_path);
    if (file_hash.size() == 0){
        // the model file cannot be loaded
        return {true, ""};
    }

    if (!fs::exists(fs::path(model_cache_path))){
        return {true, file_hash};
    }

    const std::string flag_file_path = model_cache_path + "/HASH.txt";
    if (fs::exists(fs::path(flag_file_path))){
        std::ifstream fin(flag_file_path);
        if (fin){
            std::string line;
            fin >> line;
            if (line == file_hash){
                // hash match!
                return {false, file_hash};
            }
        }
    }
    // remove everything from model_cache_path
    fs::remove_all(fs::path(model_cache_path));
    return {true, file_hash};
}


void write_cache_flag_file(const std::string& model_cache_path, const std::string& hash){
    if (!fs::exists(fs::path(model_cache_path))){
        return;
    }
    const std::string flag_file_path = model_cache_path + "/HASH.txt";
    std::ofstream fout(flag_file_path);
    fout << hash;
}


Ort::Session create_session(const Ort::Env& env, const Ort::SessionOptions& so,
    const std::string& model_path, const std::string& model_cache_path)
{
    bool write_flag_file = true;
    std::string file_hash;
    std::tie(write_flag_file, file_hash) = clean_up_old_model_cache(model_cache_path, model_path);
    
    Ort::Session session{env, str_to_onnx_str(model_path).c_str(), so};
    // when Ort::Ssssion is created, if possible, it will create a model cache
    if (write_flag_file){
        write_cache_flag_file(model_cache_path, file_hash);
    }
    return session;
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
