/*  ML Segment Anything Model
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Run Segment Anything Model (SAM) to segment objects on images
 */

#include <QDir>
#include <QDirIterator>
#include <fstream>
#include <iostream>
#include <onnxruntime_cxx_api.h>
#include <opencv2/imgcodecs.hpp>
#include "3rdParty/ONNX/OnnxToolsPA.h"
#include "SegmentAnythingModel.h"

namespace PokemonAutomation{
namespace ML{




const int SAM_EMBEDDER_INPUT_IMAGE_WIDTH = 1024;
const int SAM_EMBEDDER_INPUT_IMAGE_HEIGHT = 576;
const int SAM_EMBEDDER_OUTPUT_N_CHANNELS = 256;
const int SAM_EMBEDDER_OUTPUT_IMAGE_SIZE = 64;

const int SAM_EMBEDDER_INPUT_SIZE = SAM_EMBEDDER_INPUT_IMAGE_HEIGHT * SAM_EMBEDDER_INPUT_IMAGE_WIDTH * 3;
const int SAM_EMBEDDER_OUTPUT_SIZE = SAM_EMBEDDER_OUTPUT_N_CHANNELS * SAM_EMBEDDER_OUTPUT_IMAGE_SIZE * SAM_EMBEDDER_OUTPUT_IMAGE_SIZE;

const int SAM_N_INPUT_TENSORS = 6;
const int SAM_N_OUTPUT_TENSORS = 3;
const int SAM_LOW_RES_MASK_SIZE = 256;
const float SAM_OUTPUT_MASK_THRESHOLD = 0.0;


Ort::SessionOptions create_session_option(){
    return Ort::SessionOptions{};

    // create session using Apple ML

    // Ort::SessionOptions so;
    // std::unordered_map<std::string, std::string> provider_options;
    // provider_options["ModelFormat"] = "NeuralNetwork"; 
    // so.AppendExecutionProvider("CoreML", provider_options);
    // return so;
}


template<typename T, class Buffer, class Shape> Ort::Value create_tensor(const OrtMemoryInfo* memory_info, Buffer& buffer, const Shape& shape){
    return Ort::Value::CreateTensor<T>(memory_info, buffer.data(), buffer.size(),
        shape.data(), shape.size());
}


SAMEmbedderSession::SAMEmbedderSession(const std::string& model_path)
    : session_options(create_session_option())
    , session{env, str_to_onnx_str(model_path).c_str(), session_options}
    , memory_info{Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)}
    , input_names{session.GetInputNames()}
    , output_names{session.GetOutputNames()}
    , input_shape{1, SAM_EMBEDDER_INPUT_IMAGE_HEIGHT, SAM_EMBEDDER_INPUT_IMAGE_WIDTH, 3}
    , output_shape{1, SAM_EMBEDDER_OUTPUT_N_CHANNELS, SAM_EMBEDDER_OUTPUT_IMAGE_SIZE, SAM_EMBEDDER_OUTPUT_IMAGE_SIZE}
    , model_input(SAM_EMBEDDER_INPUT_SIZE)
{
    std::cout << "Built SAM embedder session" << std::endl;
}

void SAMEmbedderSession::run(cv::Mat& input_image, std::vector<float>& model_output){
    assert(input_image.rows == SAM_EMBEDDER_INPUT_IMAGE_HEIGHT);
    assert(input_image.cols == SAM_EMBEDDER_INPUT_IMAGE_WIDTH);

    model_output.resize(SAM_EMBEDDER_OUTPUT_SIZE);
    auto input_tensor = create_tensor<uint8_t>(memory_info, model_input, input_shape);
    auto output_tensor = create_tensor<float>(memory_info, model_output, output_shape);

    for (int row = 0, p_loc=0; row < SAM_EMBEDDER_INPUT_IMAGE_HEIGHT; row++){
        for(int col = 0; col < SAM_EMBEDDER_INPUT_IMAGE_WIDTH; col++){
            cv::Vec3b p = input_image.at<cv::Vec3b>(row, col);
            model_input[p_loc++] = p[0];
            model_input[p_loc++] = p[1];
            model_input[p_loc++] = p[2];
        }
    }

    const char* input_name_c = input_names[0].data();
    const char* output_name_c = output_names[0].data();
    auto start = std::chrono::steady_clock::now();
    session.Run(run_options, &input_name_c, &input_tensor, 1, &output_name_c, &output_tensor, 1);
    auto end = std::chrono::steady_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Embedder inference time: " << milliseconds << " ms" << std::endl;
}


SAMSession::SAMSession(const std::string& model_path)
    : session_options(create_session_option())
    , session{env, str_to_onnx_str(model_path).c_str(), session_options}
    , memory_info{Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)}
    , input_names{session.GetInputNames()}
    , output_names{session.GetOutputNames()}
    , input_image_embedding_shape{1, SAM_EMBEDDER_OUTPUT_N_CHANNELS,
        SAM_EMBEDDER_OUTPUT_IMAGE_SIZE, SAM_EMBEDDER_OUTPUT_IMAGE_SIZE}
    , input_mask_shape{1, 1, SAM_LOW_RES_MASK_SIZE, SAM_LOW_RES_MASK_SIZE}
    , output_low_res_mask_shape{1, 1, SAM_LOW_RES_MASK_SIZE, SAM_LOW_RES_MASK_SIZE}
    , input_mask_buffer(SAM_LOW_RES_MASK_SIZE * SAM_LOW_RES_MASK_SIZE, 0.0)
    , output_low_res_mask_buffer(SAM_LOW_RES_MASK_SIZE * SAM_LOW_RES_MASK_SIZE, 0.0)
{
    std::cout << "Built SAM session" << std::endl;
}

void SAMSession::run(
    std::vector<float>& image_embedding,
    int original_image_height, int original_image_width,
    const std::vector<int>& input_points,
    const std::vector<int>& input_point_labels,
    const std::vector<int>& input_box,
    std::vector<bool>& output_boolean_mask
){
    assert(image_embedding.size() == SAM_EMBEDDER_OUTPUT_SIZE);
    assert(input_points.size() % 2 == 0);
    assert(input_point_labels.size() == input_points.size()/2);
    assert(input_box.size() == 0 || input_box.size() == 4);  // 4 is x_min, y_min, x_max, y_max
    assert(input_points.size() > 0 || input_box.size() > 0);

    size_t num_points = input_points.size() / 2;
    if(input_box.size() > 0){
        num_points += 2; // add the bounding box two corners
    }
    else{
        num_points += 1; // add a padding point where there is no bounding box
    }

    input_point_coords_shape[1] = num_points;
    input_point_coords_buffer.clear();
    // padding point coords are 0.0
    input_point_coords_buffer.resize(num_points * 2, 0.0);

    input_point_labels_shape[1] = num_points;
    input_point_labels_buffer.clear();
    // `point_labels`: Labels for the sparse input prompts. 0 is a negative input point, 1 is a positive input point,
    // 2 is a top-left box corner, 3 is a bottom-right box corner, and -1 is a padding point. 
    // If there is no box input, a single padding point with label -1 and coordinates (0.0, 0.0) should be concatenated.
    input_point_labels_buffer.resize(num_points, -1.0);
    input_orig_im_size_buffer[0] = float(original_image_height);
    input_orig_im_size_buffer[1] = float(original_image_width);

    // assign input coordinates and boxes:
    const float scale_x = SAM_EMBEDDER_INPUT_IMAGE_WIDTH / float(original_image_width);
    const float scale_y = SAM_EMBEDDER_INPUT_IMAGE_HEIGHT / float(original_image_height);
    for(size_t i = 0; i < input_point_labels.size(); i++){
        input_point_coords_buffer[2*i] = input_points[2*i] * scale_x;
        input_point_coords_buffer[2*i+1] = input_points[2*i+1] * scale_y;

        input_point_labels_buffer[i] = float(input_point_labels[i]);
    }
    if (input_box.size() > 0){
        // assign input box label
        input_point_coords_buffer[2*input_point_labels.size()] = input_box[0] * scale_x;
        input_point_coords_buffer[2*input_point_labels.size()+1] = input_box[1] * scale_y;
        input_point_coords_buffer[2*input_point_labels.size()+2] = input_box[2] * scale_x;
        input_point_coords_buffer[2*input_point_labels.size()+3] = input_box[3] * scale_y;

        input_point_labels_buffer[input_point_labels.size()] = 2;
        input_point_labels_buffer[input_point_labels.size()+1] = 3;
    }

    output_mask_shape[2] = original_image_height;
    output_mask_shape[3] = original_image_width;
    output_mask_buffer.clear();
    output_mask_buffer.resize(original_image_height * original_image_width, 0.0);

    std::array<Ort::Value, SAM_N_INPUT_TENSORS> input_tensors;
    input_tensors[0] = create_tensor<float>(memory_info, image_embedding, input_image_embedding_shape);
    input_tensors[1] = create_tensor<float>(memory_info, input_point_coords_buffer, input_point_coords_shape);
    input_tensors[2] = create_tensor<float>(memory_info, input_point_labels_buffer, input_point_labels_shape);
    input_tensors[3] = create_tensor<float>(memory_info, input_mask_buffer, input_mask_shape);
    input_tensors[4] = create_tensor<float>(memory_info, input_has_mask_buffer, input_has_mask_shape);
    input_tensors[5] = create_tensor<float>(memory_info, input_orig_im_size_buffer, input_orig_im_size_shape);
    std::array<Ort::Value, SAM_N_OUTPUT_TENSORS> output_tensors;
    output_tensors[0] = create_tensor<float>(memory_info, output_mask_buffer, output_mask_shape);
    output_tensors[1] = create_tensor<float>(memory_info, output_iou_prediction_buffer, output_iou_prediction_shape);
    output_tensors[2] = create_tensor<float>(memory_info, output_low_res_mask_buffer, output_low_res_mask_shape);

    std::array<const char*, SAM_N_INPUT_TENSORS> input_names_c;
    for(int i = 0; i < SAM_N_INPUT_TENSORS; i++){
        input_names_c[i] = input_names[i].data();
    }
    std::array<const char*, SAM_N_OUTPUT_TENSORS> output_names_c;
    for(int i = 0; i < SAM_N_OUTPUT_TENSORS; i++){
        output_names_c[i] = output_names[i].data();
    }

    auto start = std::chrono::steady_clock::now();
    session.Run(run_options, input_names_c.data(), input_tensors.data(), SAM_N_INPUT_TENSORS,
        output_names_c.data(), output_tensors.data(), SAM_N_OUTPUT_TENSORS);
    auto end = std::chrono::steady_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "SAM inference time: " << milliseconds << " ms" << std::endl;

    output_boolean_mask.resize(original_image_height * original_image_width, false);
    for(size_t i = 0; i < output_mask_buffer.size(); i++){
        output_boolean_mask[i] = output_mask_buffer[i] > SAM_OUTPUT_MASK_THRESHOLD;
    }
}


// save the image embedding as a file with path <image_filepath>.embedding
void save_image_embedding_to_disk(const std::string& image_filepath, const std::vector<float>& embedding){
    std::string embedding_path = image_filepath + ".embedding";
    std::ofstream fout(embedding_path, std::ios::binary);
    // write embedding shape
    fout.write(reinterpret_cast<const char*>(&SAM_EMBEDDER_OUTPUT_N_CHANNELS), sizeof(SAM_EMBEDDER_OUTPUT_N_CHANNELS));
    fout.write(reinterpret_cast<const char*>(&SAM_EMBEDDER_OUTPUT_IMAGE_SIZE), sizeof(SAM_EMBEDDER_OUTPUT_IMAGE_SIZE));
    fout.write(reinterpret_cast<const char*>(&SAM_EMBEDDER_OUTPUT_IMAGE_SIZE), sizeof(SAM_EMBEDDER_OUTPUT_IMAGE_SIZE));
    fout.write(reinterpret_cast<const char*>(embedding.data()), sizeof(float) * embedding.size());
    fout.close();
    std::cout << "Saved image embedding as " << embedding_path << std::endl;
}


bool load_image_embedding(const std::string& image_filepath, std::vector<float>& image_embedding){
    std::string emebdding_path = image_filepath + ".embedding";
    std::ifstream fin(emebdding_path, std::ios::binary);
    if (!fin.is_open()){
        std::cout << "No embedding for image " << image_filepath << std::endl;
        return false;
    }

    int embedding_n_channels = 0, embedding_height = 0, emebedding_width = 0;
    fin.read(reinterpret_cast<char*>(&embedding_n_channels), sizeof(int));
    fin.read(reinterpret_cast<char*>(&embedding_height), sizeof(int));
    fin.read(reinterpret_cast<char*>(&emebedding_width), sizeof(int));

    std::cout << "Image embedding shape [" << embedding_n_channels << ", " << embedding_height
              << ", " << emebedding_width << "]" << std::endl;
    if (embedding_n_channels <= 0 || embedding_height <= 0 || emebedding_width <= 0){
        std::string err_msg = "Image embedding wrong dimension from " + emebdding_path;
        std::cerr << err_msg << std::endl;
        throw std::runtime_error(err_msg);
    }

    const int size = embedding_n_channels * embedding_height * emebedding_width;
    image_embedding.resize(size);
    fin.read(reinterpret_cast<char*>(image_embedding.data()), sizeof(float) * size);
    std::cout << "Loaded image embedding from " << emebdding_path << std::endl;
    return true;
}


void compute_embeddings_for_folder(const std::string& image_folder_path){
    QDir image_dir(image_folder_path.c_str());
    if (!image_dir.exists()){
        std::cerr << "Error: input image folder path " << image_folder_path << " does not exist." << std::endl;
        return;
    }

    QDirIterator image_file_iter(image_dir.absolutePath(), {"*.png", "*.jpg", "*.jpeg"}, QDir::Files, QDirIterator::Subdirectories);
    std::vector<std::string> all_image_paths;
    while (image_file_iter.hasNext()){
        all_image_paths.emplace_back(image_file_iter.next().toStdString());
    }
    std::cout << "Found " << all_image_paths.size() << " images recursively in folder " << image_folder_path << std::endl;
}

}
}
