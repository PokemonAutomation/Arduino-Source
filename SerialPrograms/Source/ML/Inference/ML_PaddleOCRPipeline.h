/*  ML Paddle OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  
 */

#ifndef PokemonAutomation_ML_PaddleOCRPipeline_H
#define PokemonAutomation_ML_PaddleOCRPipeline_H

#include <memory>
#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Logging/TaggedLogger.h"
#include "Common/Cpp/Filesystem/FilePath.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "ML_PaddleOCRDetector.h"

namespace PokemonAutomation{
namespace ML{


// One piece of text found by `PaddleOCRPipeline::detect_and_recognize()`.
struct PaddleOCRTextResult{
    // Recognized text of this box. May be empty if recognition found nothing.
    std::string text;
    // Where the text is, in pixel coordinates of the input image.
    PaddleOCRTextBox box;
    // Index of the text line this box belongs to, counting from the top (0-based).
    // Several boxes can share one line, e.g. two words far apart on the same row.
    size_t line_index = 0;
};


// Full PaddleOCR pipeline: text detection (PP-OCRv5 det, DBNet) + text recognition
// (PP-OCRv5 rec, CTC).
//
// There are two ways to use it:
// - `recognize()`: single-line OCR. Assumes the image is already cropped around one
//   line of text. It skips the detection model and instead tightens and pads the crop
//   using a simple Otsu threshold (`crop_to_text_region_with_padding()`). This is fast
//   and is what the majority of our game readers use.
// - `detect_and_recognize()` / `recognize_multiline()`: arbitrary images containing
//   any number of lines. Runs the detection model to find each text region, then runs
//   the recognition model on each region.
//
// The detection model file is the same for all languages. It is loaded lazily on first
// use, so single-line only users never pay for it.
// All public methods are thread-safe.
class PaddleOCRPipeline{
public:
    PaddleOCRPipeline(Language language);
    PaddleOCRPipeline(
        Language language,
        std::string det_path,
        std::string rec_path,
        std::string dict_path
    );

    // Single-line OCR on an image pre-cropped to one line of text.
    std::string recognize(const ImageViewRGB32& image);

    // Multi-line OCR: detect all text regions in `image` and recognize each of them.
    // Results are in reading order: top to bottom, then left to right within a line.
    // Throws FileException if the detection model has not been downloaded, or
    // `MLModelSessionCreationError` if it cannot be loaded.
    std::vector<PaddleOCRTextResult> detect_and_recognize(const ImageViewRGB32& image);

    // Multi-line OCR returning plain text: each detected line is one line of the
    // result, joined by '\n'. Boxes on the same line are joined by a space, except
    // for Chinese and Japanese which do not use spaces between words.
    // e.g. a two line dialog box gives "You hurry to the Pokemon Center, shielding your\neyes from the rain."
    std::string recognize_multiline(const ImageViewRGB32& image);

    // Get the text detector, loading the detection model if it's not loaded yet.
    // The detection model is the downloadable resource "PaddleOCRDetection".
    // Throws FileException if it has not been downloaded.
    PaddleOCRDetector& detector();

    // Return the (recognition model path, dictionary path) for a language.
    static std::pair<std::string, std::string> get_paths(Language language);

    std::string decode_CTC(float* data, const std::vector<int64_t>& shape, const std::vector<std::string>& dict);

private:
    void load_dictionary(const Filesystem::Path& path);

    // Run the recognition model on an RGB image (CV_8UC3) that is already cropped
    // tightly around a single line of text. Resizes it to the model height, normalizes
    // it, runs the model and CTC-decodes the output.
    std::string recognize_line(const cv::Mat& line_rgb);

    Ort::Session m_rec_session;
    Language m_language;
    std::string m_input_name;
    std::string m_output_name;
    std::vector<std::string> m_dictionary;
    TaggedLogger m_logger;

    int m_index = 0;

    std::string m_det_path;
    Mutex m_detector_lock;
    std::unique_ptr<PaddleOCRDetector> m_detector;
};

// assumes the input image is RGB
cv::Mat crop_to_text_region_with_padding(const cv::Mat& image, int image_index);

// returns binary image of given image
// text pixels are white.
// uses Otsu thresholding
cv::Mat get_binary_image(const cv::Mat& image);

// if the image is narrow/tall, add horizontal padding
// modifies the input image
// assumes input image is RGB
void add_horizontal_padding(cv::Mat& image, int image_index);

// if the image is just a line, add vertical padding
// modifies the input image
// to avoid PaddleOCR hallucinating text
void add_vertical_padding(cv::Mat& image, const cv::Mat& binary_tight_crop, int image_index);

// from given binary image, returns true if it's a horizontal line
bool is_horizontal_line(const cv::Mat& binary, int image_index);

// from given binary image, and single contour within the binary image,
// return true if the given contour is line shaped. 
// density of the contour must be >75%. and at least twice as wide as it is tall
bool is_line_shape(const cv::Mat& binary, const std::vector<cv::Point>& contour);

// assumes input image is RGB
cv::Scalar estimate_background_color(const cv::Mat& image);

// convert HWC (height, width, channels) to NCHW (batch N, channels C, height H, width W)
// HWC: pixels are interleaved. [B,G,R] [B,G,R] [B,G,R] ...
// NCHW: [All Blue Pixels...] [All Green Pixels...] [All Red Pixels...]
std::vector<float> preprocess_NCHW(cv::Mat& img);


cv::Mat imageviewrgb32_to_cv_mat_rgb(const ImageViewRGB32& image);

cv::Rect ImageFloatBox_to_cv_Rect(size_t width, size_t height, const ImageFloatBox& box);


}
}
#endif
