/*  FRLG OCR Preprocessing
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32_OpenCV.h"

#include "PokemonFRLG_OcrPreprocessing.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


ImageRGB32 preprocess_for_ocr(
    const ImageViewRGB32& image,
    int blur_kernel_size, int blur_passes
){
    if (!image || image.width() == 0 || image.height() == 0){
        return ImageRGB32();
    }

    // Rescale the image to match the expected height at 1080p
    // so that the Gaussian blur size works across resolutions
    constexpr int expected_height = 69;

    double scale_factor = static_cast<double>(expected_height) / static_cast<double>(image.height());

    int new_w = std::max(
        1, cvRound(static_cast<int>(image.width() * scale_factor))
    );

    int new_h = std::max(
        1, cvRound(static_cast<int>(image.height() * scale_factor))
    );

    ImageRGB32 blurred = image.scale_to(new_w, new_h);

    // The 5x5 kernel reaches 2 pixels away (vs 1px for 3x3), bridging
    // wider gaps in the seven-segment font. Two passes for heavy smoothing.
    cv::Mat mat = to_OpenCV_ref(blurred);
    if (blur_kernel_size > 0 && blur_passes > 0){
        for (int i = 0; i < blur_passes; i++){
            cv::GaussianBlur(
                mat, mat,
                cv::Size(blur_kernel_size, blur_kernel_size), 1.5
            );
        }
    }

    return blurred;
}


const std::vector<OCR::TextColorRange>& DARK_TEXT_FILTERS(){
    static std::vector<OCR::TextColorRange> filters{
        {combine_rgb(0, 0, 0), combine_rgb(96, 96, 96)},
        {combine_rgb(0, 0, 0), combine_rgb(128, 128, 128)},
        {combine_rgb(0, 0, 0), combine_rgb(160, 160, 160)},
        {combine_rgb(0, 0, 0), combine_rgb(190, 190, 190)},
    };
    return filters;
}

const std::vector<OCR::TextColorRange>& BRIGHT_TEXT_FILTERS(){
    static std::vector<OCR::TextColorRange> filters{
        {combine_rgb(180, 180, 180), combine_rgb(255, 255, 255)},
        {combine_rgb(200, 200, 200), combine_rgb(255, 255, 255)},
        {combine_rgb(220, 220, 220), combine_rgb(255, 255, 255)},
        {combine_rgb(230, 230, 230), combine_rgb(255, 255, 255)},
        {combine_rgb(240, 240, 240), combine_rgb(255, 255, 255)},
    };
    return filters;
}


} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
