/*  ML Paddle OCR Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Text detection half of the PaddleOCR pipeline. Given an arbitrary image, it finds
 *  where the lines of text are. Each detected region can then be cropped out and fed
 *  into the recognition model (see `PaddleOCRPipeline`).
 *
 *  The detection model is PP-OCRv5 det, a DBNet ("Differentiable Binarization") model.
 *  Model source: https://huggingface.co/monkt/paddleocr-onnx/tree/main/detection/v5
 *  The same detection model is shared by all languages.
 */

#ifndef PokemonAutomation_ML_PaddleOCRDetector_H
#define PokemonAutomation_ML_PaddleOCRDetector_H

#include <array>
#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h>
#include "Common/Cpp/Rectangle.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace cv{
    class Mat;
}

namespace PokemonAutomation{
namespace ML{


// Tunable parameters of the DBNet detection pre/post-processing.
// Defaults follow PaddleOCR's PP-OCRv5 `DetResizeForTest` and `DBPostProcess` configs,
// except where noted.
struct PaddleOCRDetectorParams{
    // Preprocessing: if the shorter image side is smaller than this, upscale the image
    // so that the shorter side matches it. Tiny images give the model too few pixels
    // to find text in.
    int min_side_len = 64;
    // Preprocessing: if the longer image side is larger than this, downscale the image
    // so that the longer side matches it. This bounds the inference cost for large
    // images such as full 4K screenshots.
    int max_side_len = 2048;
    // Preprocessing: pad the image by this many pixels on each side (in the original
    // image scale) using the estimated background color. DBNet tends to miss text that
    // touches the image border, which is common for our pre-cropped OCR boxes.
    // (Not part of PaddleOCR.)
    int border_padding = 8;

    // Postprocessing: pixels whose text probability is above this are considered text.
    float binary_threshold = 0.3f;
    // Postprocessing: drop candidate boxes whose mean probability is below this.
    float box_threshold = 0.6f;
    // Postprocessing: how much to expand the shrunk text kernel back out.
    // DBNet is trained to predict a shrunk version of each text region, so the found
    // boxes must be "unclipped" (expanded) to cover the whole text.
    float unclip_ratio = 1.5f;
    // Postprocessing: drop boxes whose shorter side is smaller than this (in pixels of
    // the model input).
    float min_box_size = 3.0f;
    // Postprocessing: maximum number of contours to consider.
    size_t max_candidates = 1000;
};


// A point in (sub-)pixel image coordinates: x from left to right, y from top to bottom.
struct PaddleOCRPoint{
    float x = 0;
    float y = 0;
};

// One detected text region.
struct PaddleOCRTextBox{
    // Four corners of the (possibly slightly rotated) text rectangle in pixel
    // coordinates of the original input image. Order: top-left, top-right,
    // bottom-right, bottom-left.
    std::array<PaddleOCRPoint, 4> corners;
    // Mean text probability inside the box, in [0, 1].
    float score = 0;

    // Axis-aligned extent of `corners`, in sub-pixel image coordinates.
    Rectangle<float> extent() const;

    // Smallest pixel box enclosing `corners`, clipped to the image size.
    ImagePixelBox pixel_box(size_t image_width, size_t image_height) const;
};


class PaddleOCRDetector{
public:
    // Load the detection model from `model_path`.
    // Throws `MLModelSessionCreationError` if the ONNX session cannot be created.
    PaddleOCRDetector(const std::string& model_path, PaddleOCRDetectorParams params = {});

    // Detect text regions in an RGB image (CV_8UC3). Returns boxes in unspecified order.
    // Returns an empty vector if the image is empty or contains no text.
    // Thread-safe: ONNX Runtime sessions can be run concurrently.
    // Throws InternalProgramError if ONNX Runtime fails.
    std::vector<PaddleOCRTextBox> detect(const cv::Mat& image_rgb) const;

    const PaddleOCRDetectorParams& params() const{ return m_params; }

private:
    // Run the DBNet postprocessing on the model's output probability map.
    // `prob` is a CV_32F map of the model input size. `scale_x`, `scale_y` map model
    // input pixel coordinates back to original image coordinates, after subtracting
    // `offset` (the border padding, in original image pixels).
    std::vector<PaddleOCRTextBox> postprocess(
        const cv::Mat& prob,
        float scale_x, float scale_y,
        float offset,
        int image_width, int image_height
    ) const;

private:
    PaddleOCRDetectorParams m_params;
    mutable Ort::Session m_session;
    std::string m_input_name;
    std::string m_output_name;
};


// Sort detected text boxes into reading order and group them into lines.
// Boxes whose vertical extents overlap by at least half of the shorter box height are
// put into the same line. Lines are ordered top to bottom, boxes within a line left to
// right. Returns indices into `boxes`: result[line][k].
std::vector<std::vector<size_t>> group_text_boxes_into_lines(const std::vector<PaddleOCRTextBox>& boxes);

// Merge the boxes of one text line (`line_boxes`, sorted left to right) into
// horizontal segments. Neighboring boxes are merged when the horizontal gap between
// them is at most `max_gap_ratio` times the line height. Each merged segment is an
// axis-aligned box spanning the full height of the line.
//
// Why: DBNet often splits a line into separate words, especially with widely spaced
// pixel fonts like FRLG. Each word box then has its own height: "see" only covers the
// lowercase x-height while "BULBASAUR" is taller. After scaling every crop to the
// recognition model's fixed input height, short words get oversized glyphs and are
// misread (e.g. "see" -> "SPP"). Recognizing the whole line at a consistent height
// fixes this, and the recognition model outputs the spaces between words itself.
// Distant boxes (e.g. separate table columns) are kept apart.
//
// Rotated boxes (more than a few degrees) are never merged, and are returned as is.
std::vector<PaddleOCRTextBox> merge_text_boxes_in_line(
    const std::vector<PaddleOCRTextBox>& line_boxes,
    float max_gap_ratio = 1.0f
);

// Crop the text region `box` from `image_rgb` and warp it into an axis-aligned
// rectangle, so slightly rotated text becomes horizontal. This is PaddleOCR's
// `get_rotate_crop_image()`, minus the auto 90-degree rotation of tall crops (game UI
// text is never vertical, while narrow single characters like "1" would be wrongly
// rotated). Returns an empty Mat if the box is degenerate.
cv::Mat crop_text_box(const cv::Mat& image_rgb, const PaddleOCRTextBox& box);


}
}
#endif
