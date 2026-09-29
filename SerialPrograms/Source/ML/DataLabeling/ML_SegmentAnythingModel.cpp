/*  ML Segment Anything Model
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Run Segment Anything Model (SAM) to segment objects on images
 */

#include <QDir>
#include <QDirIterator>
#include <atomic>
#include <format>
#include <fstream>
#include <mutex>
#include <iostream>
#include <QMessageBox>
#include <onnxruntime_cxx_api.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include "3rdParty/ONNX/OnnxToolsPA.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "ML/Models/ML_ONNXRuntimeHelpers.h"
#include "ML_SegmentAnythingModelConstants.h"
#include "ML_SegmentAnythingModel.h"
#include "ML_AnnotationIO.h"

namespace PokemonAutomation{
namespace ML{


SAMEmbedderSession::SAMEmbedderSession(const std::string& model_path, bool use_gpu)
    : session{create_session(model_path, ML_MODEL_CACHE_PATH() + "SAMEmbedder/", use_gpu)}
    , memory_info{Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)}
    , input_names{session.GetInputNames()}
    , output_names{session.GetOutputNames()}
    , input_shape{1, SAM_EMBEDDER_INPUT_IMAGE_HEIGHT, SAM_EMBEDDER_INPUT_IMAGE_WIDTH, 3}
    , output_shape{1, SAM_EMBEDDER_OUTPUT_N_CHANNELS, SAM_EMBEDDER_OUTPUT_IMAGE_SIZE, SAM_EMBEDDER_OUTPUT_IMAGE_SIZE}
{
    std::cout << "Built SAM embedder session" << std::endl;
}

void SAMEmbedderSession::run(const cv::Mat& input_image, std::vector<float>& model_output){
    assert(input_image.rows == SAM_EMBEDDER_INPUT_IMAGE_HEIGHT);
    assert(input_image.cols == SAM_EMBEDDER_INPUT_IMAGE_WIDTH);

    // The input buffer is local (not a member) so that multiple threads can call `run()` on the same
    // session concurrently. `Ort::Session::Run()` itself is thread-safe.
    std::vector<uint8_t> model_input(SAM_EMBEDDER_INPUT_SIZE);
    model_output.resize(SAM_EMBEDDER_OUTPUT_SIZE);
    auto input_tensor = create_tensor<uint8_t>(memory_info, model_input, input_shape);
    auto output_tensor = create_tensor<float>(memory_info, model_output, output_shape);

    for (int row = 0, p_loc=0; row < SAM_EMBEDDER_INPUT_IMAGE_HEIGHT; row++){
        for (int col = 0; col < SAM_EMBEDDER_INPUT_IMAGE_WIDTH; col++){
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


SAMSession::SAMSession(const std::string& model_path, bool use_gpu)
    : session{create_session(model_path, ML_MODEL_CACHE_PATH() + "SAM/", use_gpu)}
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
    }else{
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
    for (size_t i = 0; i < input_point_labels.size(); i++){
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
    for (int i = 0; i < SAM_N_INPUT_TENSORS; i++){
        input_names_c[i] = input_names[i].data();
    }
    std::array<const char*, SAM_N_OUTPUT_TENSORS> output_names_c;
    for (int i = 0; i < SAM_N_OUTPUT_TENSORS; i++){
        output_names_c[i] = output_names[i].data();
    }

    auto start = std::chrono::steady_clock::now();
    session.Run(run_options, input_names_c.data(), input_tensors.data(), SAM_N_INPUT_TENSORS,
        output_names_c.data(), output_tensors.data(), SAM_N_OUTPUT_TENSORS);
    auto end = std::chrono::steady_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "SAM inference time: " << milliseconds << " ms" << std::endl;

    output_boolean_mask.resize(original_image_height * original_image_width, false);
    for (size_t i = 0; i < output_mask_buffer.size(); i++){
        output_boolean_mask[i] = output_mask_buffer[i] > SAM_OUTPUT_MASK_THRESHOLD;
    }
}


namespace{

// Result of trying to compute the embedding of one image in `embed_one_image()`.
enum class EmbedImageResult{
    SUCCESS,
    FAILED_TO_LOAD,         // `cv::imread()` returned an empty image, e.g. the file is not an actual image.
    UNSUPPORTED_CHANNELS,   // image is not 3-channel (BGR) or 4-channel (BGRA).
};

// Load the image at `image_path`, convert it to the RGB, SAM_EMBEDDER_INPUT_IMAGE_WIDTH x
// SAM_EMBEDDER_INPUT_IMAGE_HEIGHT format that `SAMEmbedderSession::run()` expects, run the embedder
// and save the result to "<image_path>.embedding" via `save_image_embedding_to_disk()`.
// This is the per-image work item of `compute_embeddings_for_folder()` and runs on worker threads
// of `GlobalThreadPools::computation_normal()`, so it must not touch any Qt UI (e.g. QMessageBox).
// Image problems are reported through the return value. Inference failures propagate as exceptions
// (usually `Ort::Exception`) so the caller can decide whether to fall back from GPU to CPU.
EmbedImageResult embed_one_image(SAMEmbedderSession& embedding_session, const std::string& image_path){
    cv::Mat image_bgr = cv::imread(image_path);
    if (image_bgr.empty()){
        return EmbedImageResult::FAILED_TO_LOAD;
    }
    cv::Mat image;
    if (image_bgr.channels() == 4){
        cv::cvtColor(image_bgr, image, cv::COLOR_BGRA2RGB);
    }else if (image_bgr.channels() == 3){
        cv::cvtColor(image_bgr, image, cv::COLOR_BGR2RGB);
    }else{
        return EmbedImageResult::UNSUPPORTED_CHANNELS;
    }

    cv::Mat resized_mat;  // resize to the shape for the ML model input
    cv::resize(image, resized_mat, cv::Size(SAM_EMBEDDER_INPUT_IMAGE_WIDTH, SAM_EMBEDDER_INPUT_IMAGE_HEIGHT));

    std::vector<float> output_image_embedding;
    embedding_session.run(resized_mat, output_image_embedding);
    save_image_embedding_to_disk(image_path, output_image_embedding);
    return EmbedImageResult::SUCCESS;
}

void log_image_list(Logger& logger, const std::string& header, const std::vector<std::string>& image_paths){
    if (image_paths.empty()){
        return;
    }
    logger.log(header, COLOR_RED);
    for (const std::string& image_path : image_paths){
        logger.log("- " + image_path, COLOR_RED);
    }
}

}


void compute_embeddings_for_folder(
    Logger& logger,
    const std::string& embedding_model_path,
    const std::string& image_folder_path,
    bool use_gpu_for_embedder_session
){
    const bool recursive_search = true;
    const std::vector<std::string> all_image_paths = find_images_in_folder(image_folder_path, recursive_search);
    if (all_image_paths.size() == 0){
        logger.log("No images found in " + image_folder_path + ".");
        return;
    }

    // Only compute embeddings that do not exist on disk yet.
    std::vector<std::string> image_paths;
    for (const std::string& image_path : all_image_paths){
        if (!Filesystem::exists(image_path + ".embedding")){
            image_paths.emplace_back(image_path);
        }
    }
    logger.log(std::format(
        "Found {} images in {}. {} already have embeddings. Computing {} embeddings...",
        all_image_paths.size(), image_folder_path,
        all_image_paths.size() - image_paths.size(), image_paths.size()
    ));
    if (image_paths.empty()){
        return;
    }

    if (!Filesystem::exists(embedding_model_path)){
        logger.log("Error: no such embedding model path " + embedding_model_path + ".", COLOR_RED);
        QMessageBox box;
        box.critical(nullptr, "Embedding Model Does Not Exist",
            QString::fromStdString("Embedding model path" + embedding_model_path + " does not exist."));
        return;
    }
    // since the embedding model has too many weights, onnx created a .data file to contain weights.
    const std::string embedding_model_data_path = embedding_model_path + ".data";
    if (!Filesystem::exists(embedding_model_data_path)){
        logger.log("Error: no such embedding model data path " + embedding_model_data_path + ".", COLOR_RED);
        QMessageBox box;
        box.critical(nullptr, "Embedding Model Data File Does Not Exist",
            QString::fromStdString("Embedding model data file path" + embedding_model_data_path + " does not exist."));
        return;
    }

    bool use_gpu = use_gpu_for_embedder_session;
    std::unique_ptr<SAMEmbedderSession> embedding_session;
    try{
        embedding_session = make_unique<SAMEmbedderSession>(embedding_model_path, use_gpu);
    }catch (MLModelSessionCreationError& e){
        QMessageBox box;
        box.warning(nullptr, "Unable To Create Model Session",
            QString::fromStdString(e.message() + ". Try using CPU?"));
        return;
    }

    // Per-image errors are collected here and reported after all images are processed, instead of
    // popping up a QMessageBox per image (which is not allowed from worker threads and would stall
    // the whole batch on one bad file).
    std::mutex error_lock;
    std::vector<std::string> images_failed_to_load;
    std::vector<std::string> images_with_unsupported_channels;
    std::vector<std::string> images_failed_inference;
    std::atomic<size_t> num_computed{0};

    auto record_result = [&](const std::string& image_path, EmbedImageResult result){
        switch (result){
        case EmbedImageResult::SUCCESS:{
            const size_t count = ++num_computed;
            logger.log(std::format("{}/{}: computed embedding for {}", count, image_paths.size(), image_path));
            return;
        }
        case EmbedImageResult::FAILED_TO_LOAD:{
            std::lock_guard<std::mutex> lg(error_lock);
            images_failed_to_load.emplace_back(image_path);
            return;
        }
        case EmbedImageResult::UNSUPPORTED_CHANNELS:{
            std::lock_guard<std::mutex> lg(error_lock);
            images_with_unsupported_channels.emplace_back(image_path);
            return;
        }
        }
    };

    // Phase 1: run serially until the first successful inference.
    // This verifies the session actually works on this machine before fanning out. If inference
    // fails on the GPU, we rebuild the session on the CPU here and retry the same image, so that
    // all the parallel workers in phase 2 share one known-good session.
    size_t next_idx = 0;
    bool session_verified = false;
    while (next_idx < image_paths.size() && !session_verified){
        const std::string& image_path = image_paths[next_idx];
        EmbedImageResult result;
        try{
            // throw Ort::Exception("Testing.", ORT_FAIL);  // to simulate GPU/CPU failure
            result = embed_one_image(*embedding_session, image_path);
        }catch (const std::exception& e){
            if (!use_gpu){
                logger.log(std::string("Error: Embedding session failed even when using the CPU.\n") + e.what(), COLOR_RED);
                QMessageBox box;
                box.warning(nullptr, "Error:",
                    QString::fromStdString("Error: Embedding session failed."));
                return;
            }
            logger.log(std::string("Warning: Embedding session failed using the GPU. Will reattempt with the CPU.\n") + e.what(), COLOR_RED);
            use_gpu = false;
            try{
                embedding_session = make_unique<SAMEmbedderSession>(embedding_model_path, use_gpu);
            }catch (MLModelSessionCreationError& e){
                QMessageBox box;
                box.warning(nullptr, "Unable To Create Model Session",
                    QString::fromStdString(e.message()));
                return;
            }
            continue;  // retry the same image with the CPU session
        }
        record_result(image_path, result);
        session_verified = result == EmbedImageResult::SUCCESS;
        next_idx++;
    }

    // Phase 2: compute the remaining embeddings in parallel.
    // `SAMEmbedderSession::run()` is thread-safe, so all workers share the same session.
    // An inference failure here only skips that image; it is reported at the end.
    GlobalThreadPools::computation_normal().run_in_parallel(
        [&](size_t image_idx){
            const std::string& image_path = image_paths[image_idx];
            try{
                record_result(image_path, embed_one_image(*embedding_session, image_path));
            }catch (const std::exception& e){
                logger.log("Error: failed to compute embedding for " + image_path + ": " + e.what(), COLOR_RED);
                std::lock_guard<std::mutex> lg(error_lock);
                images_failed_inference.emplace_back(image_path);
            }
        },
        next_idx, image_paths.size(), 1
    );

    logger.log(std::format(
        "Done computing embeddings for images in folder {}. Computed {}/{}.",
        image_folder_path, num_computed.load(), image_paths.size()
    ));
    log_image_list(logger, "Following images failed to load. Probably not actual images?", images_failed_to_load);
    log_image_list(logger, "Following images have unsupported color channels. Only work with RGB or RGBA images:", images_with_unsupported_channels);
    log_image_list(logger, "Following images failed during embedding inference:", images_failed_inference);
}

}
}
