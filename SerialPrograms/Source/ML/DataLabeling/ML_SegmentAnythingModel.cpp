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
#include <QMessageBox>
#include <onnxruntime_cxx_api.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include "3rdParty/ONNX/OnnxToolsPA.h"
#include "CommonFramework/Globals.h"
#include "ML/Models/ML_ONNXRuntimeHelpers.h"
#include "ML_SegmentAnythingModelConstants.h"
#include "ML_SegmentAnythingModel.h"
#include "ML_AnnotationIO.h"

namespace PokemonAutomation{
namespace ML{


SAMEmbedderSession::SAMEmbedderSession(const std::string& model_path)
    : m_session_options{create_session_options(ML_MODEL_CACHE_PATH() + "SAMEmbedder/")}
    , session{create_session(m_env, m_session_options, model_path, ML_MODEL_CACHE_PATH() + "SAMEmbedder/")}
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
    : m_session_options{create_session_options(ML_MODEL_CACHE_PATH() + "SAM/")}
    , session{create_session(m_env, m_session_options, model_path, ML_MODEL_CACHE_PATH() + "SAM/")}
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


void compute_embeddings_for_folder(const std::string& embedding_model_path, const std::string& image_folder_path){
    const bool recursive_search = true;
    std::vector<std::string> all_image_paths = find_images_in_folder(image_folder_path, recursive_search);
    if (all_image_paths.size() == 0){
        return;
    }
    
    if (!std::filesystem::exists(embedding_model_path)){
        std::cerr << "Error: no such embedding model path " << embedding_model_path << "." << std::endl;
        QMessageBox box;
        box.critical(nullptr, "Embedding Model Does Not Exist",
            QString::fromStdString("Embedding model path" + embedding_model_path + " does not exist."));
        return;
    }
    // since the embedding model has too many weights, onnx created a .data file to contain weights.
    auto embedding_model_data_path = embedding_model_path + ".data";
    if (!std::filesystem::exists(embedding_model_data_path)){
        std::cerr << "Error: no such embedding model data path " << embedding_model_data_path << "." << std::endl;
        QMessageBox box;
        box.critical(nullptr, "Embedding Model Data File Does Not Exist",
            QString::fromStdString("Embedding model data file path" + embedding_model_data_path + " does not exist."));
        return;
    }

    SAMEmbedderSession embedding_session(embedding_model_path);
    std::vector<float> output_image_embedding;
    for (size_t i = 0; i < all_image_paths.size(); i++){
        const auto& image_path = all_image_paths[i];
        std::cout << (i+1) << "/" << all_image_paths.size() << ": ";
        const std::string embedding_path = image_path + ".embedding";
        if (std::filesystem::exists(embedding_path)){
            std::cout << "skip already computed embedding " << embedding_path << "." << std::endl;
            continue;
        }
        std::cout << "computing embedding for " << image_path << "..." << std::endl;
        cv::Mat image_bgr = cv::imread(image_path);
        if (image_bgr.empty()){
            std::cerr << "Error: image empty. Probably the file is not an image?" << std::endl;
            QMessageBox box;
            box.warning(nullptr, "Unable To Open Image",
                QString::fromStdString("Cannot open image file " + image_path + ". Probably not an actual image?"));
            return;
        }
        cv::Mat image;
        if (image_bgr.channels() == 4){
            cv::cvtColor(image_bgr, image, cv::COLOR_BGRA2RGB);
        } else if (image_bgr.channels() == 3){
            cv::cvtColor(image_bgr, image, cv::COLOR_BGR2RGB);
        } else{
            std::cerr << "Error: wrong image channels. Only work with RGB or RGBA images." << std::endl;
            QMessageBox box;
            box.warning(nullptr, "Wrong Image Channels",
                QString::fromStdString("Image has " + std::to_string(image_bgr.channels()) + " channels. Only support 3 or 4 channels."));
            return;
        }

        cv::Mat resized_mat;  // resize to the shape for the ML model input
        cv::resize(image, resized_mat, cv::Size(SAM_EMBEDDER_INPUT_IMAGE_WIDTH, SAM_EMBEDDER_INPUT_IMAGE_HEIGHT));

        output_image_embedding.clear();
        embedding_session.run(resized_mat, output_image_embedding);
        save_image_embedding_to_disk(image_path, output_image_embedding);
    }
    std::cout << "Done computing embeddings for images in folder " << image_folder_path << "." << std::endl;

}

}
}
