/*  ML Segment Anything Model
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Run Segment Anything Model (SAM) to segment objects on images
 */

#ifndef PokemonAutomation_ML_SegmentAnythingModel_H
#define PokemonAutomation_ML_SegmentAnythingModel_H


#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h>

namespace cv{
    class Mat;
}

namespace PokemonAutomation{
namespace ML{


// Compute embeddings for all images in a folder. Only support .png, .jpg and .jpeg filename extensions so far.
// This can be very slow!
void compute_embeddings_for_folder(const std::string& embedding_model_path, const std::string& image_folder_path);


class SAMEmbedderSession{
public:
    SAMEmbedderSession(const std::string& model_path);

    // Given an image of shape SAM_EMBEDDER_INPUT_IMAGE_WIDTH x SAM_EMBEDDER_INPUT_IMAGE_HEIGHT, RGB channel order,
    // compute its image embedding as a vector<float> of size [SAM_EMBEDDER_OUTPUT_SIZE]
    // it has shape [1, SAM_EMBEDDER_OUTPUT_N_CHANNELS, SAM_EMBEDDER_OUTPUT_IMAGE_SIZE, SAM_EMBEDDER_OUTPUT_IMAGE_SIZE]
    void run(cv::Mat& input_image, std::vector<float>& output_image_embedding);
    
private:
    Ort::Env m_env;
    Ort::SessionOptions m_session_options;
    Ort::Session session;
    Ort::MemoryInfo memory_info;
    Ort::RunOptions run_options;
    std::vector<std::string> input_names, output_names;

    const std::array<int64_t, 4> input_shape;
    const std::array<int64_t, 4> output_shape;

    std::vector<uint8_t> model_input;
};

// Run Segment Anything Model in an ONNX session.
class SAMSession{
public:
    SAMSession(const std::string& model_path);

    // embedding: input embedding
    // input_points: input point coordinates (x, y) in pixel units. [p0_x, p0_y, p1_x, p1_y, p2_x, ...].
    //     Vector size: 2*num_points
    // input_point_labels: if a point is part of the object to segment, its corresponding label value is 1.
    //     if a point is outside of the object, value is 0. Vector size: num_points.
    // input_box: if not empty, the two corner points (in pixel units) of a bounding box for the object to segment.
    //     [p0_x, p0_y, p1_x, p1_y], where p0 is the top-left corner and p1 is the lower right corner.
    // output_boolean_mask: output mask in the shape of [original_image_height x original_image_width].
    //     Vector size: original_image_height * original_image_width.
    void run(
        std::vector<float>& embedding,
        int original_image_height, int original_image_width,
        const std::vector<int>& input_points,
        const std::vector<int>& input_point_labels,
        const std::vector<int>& input_box,
        std::vector<bool>& output_boolean_mask);
private:
    Ort::Env m_env;
    Ort::SessionOptions m_session_options;
    Ort::Session session;
    Ort::MemoryInfo memory_info;
    Ort::RunOptions run_options;
    std::vector<std::string> input_names, output_names;

    const std::array<int64_t, 4> input_image_embedding_shape;
    std::array<int64_t, 3> input_point_coords_shape{1, -1, 2};
    std::array<int64_t, 2> input_point_labels_shape{1, -1};
    const std::array<int64_t, 4> input_mask_shape;
    const std::array<int64_t, 1> input_has_mask_shape{1};
    const std::array<int64_t, 1> input_orig_im_size_shape{2};
    std::array<int64_t, 4> output_mask_shape{1, 1, -1, -1};
    const std::array<int64_t, 2> output_iou_prediction_shape{1, 1};
    const std::array<int64_t, 4> output_low_res_mask_shape;

    std::vector<float> input_point_coords_buffer;
    std::vector<float> input_point_labels_buffer;
    std::vector<float> input_mask_buffer;
    std::array<float, 1> input_has_mask_buffer{0.0};
    std::array<float, 2> input_orig_im_size_buffer;
    std::vector<float> output_mask_buffer;
    std::array<float, 1> output_iou_prediction_buffer{0.0};
    std::vector<float> output_low_res_mask_buffer; 
};



}
}
#endif