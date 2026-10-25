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
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "CommonTools/Images/ImageFilter.h"

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
        {combine_rgb(0, 0, 0), combine_rgb(184, 184, 184)},
        {combine_rgb(0, 0, 0), combine_rgb(208, 208, 208)},
    };
    return filters;
}

const std::vector<OCR::TextColorRange>& BRIGHT_TEXT_FILTERS(){
    static std::vector<OCR::TextColorRange> filters{
        {combine_rgb(180, 180, 180), combine_rgb(255, 255, 255)},
        {combine_rgb(190, 190, 190), combine_rgb(255, 255, 255)},
        {combine_rgb(200, 200, 200), combine_rgb(255, 255, 255)},
        {combine_rgb(210, 210, 210), combine_rgb(255, 255, 255)},
        {combine_rgb(220, 220, 220), combine_rgb(255, 255, 255)},
        {combine_rgb(230, 230, 230), combine_rgb(255, 255, 255)},
        {combine_rgb(240, 240, 240), combine_rgb(255, 255, 255)},
    };
    return filters;
}


std::string read_nidoran_symbol(
    const ImageViewRGB32& name, const std::vector<OCR::TextColorRange>& text_color_ranges
){
    // OCR drops the monochrome gender suffix, so match its shape separately.
    for (const auto& filter : text_color_ranges){
        const ImageRGB32 filtered = filter_rgb32_range(name, filter.mins, filter.maxs, Color(0), false);
        const ImageViewRGB32 text = ImageMatch::trim_image_alpha(filtered);
        if (!text){
            continue;
        }

        // Isolate the last glyph by the empty column separating it from the name.
        size_t left = text.width();
        while (left > 0){
            bool foreground = false;
            for (size_t y = 0; y < text.height(); y++){
                if (Color(text.pixel(left - 1, y)).alpha() != 0){
                    foreground = true;
                    break;
                }
            }
            if (!foreground){
                break;
            }
            left--;
        }
        const ImageViewRGB32 symbol = ImageMatch::trim_image_alpha(
            text.sub_image(left, 0, text.width() - left, text.height())
        );
        const double aspect_ratio = (double)symbol.width() / symbol.height();
        if (aspect_ratio < 0.4 || aspect_ratio > 0.65){
            continue;
        }

        // Compare the FRLG font's 5 x 10 gender glyphs after removing the background.
        static constexpr const char* symbols[] = {
            ".###." "#...#" "#...#" "#...#" ".###." "..#.." "#####" "..#.." "..#.." "..#..",
            "..#.." ".###." "#.#.#" "..#.." "..#.." ".###." "#...#" "#...#" "#...#" ".###."
        };
        const ImageRGB32 scaled = symbol.scale_to(5, 10);
        double errors[2] = {};
        for (size_t y = 0; y < 10; y++){
            for (size_t x = 0; x < 5; x++){
                const uint8_t alpha = Color(scaled.pixel(x, y)).alpha();
                for (size_t gender = 0; gender < 2; gender++){
                    errors[gender] += symbols[gender][y * 5 + x] == '#' ? 255 - alpha : alpha;
                }
            }
        }
        for (double& error : errors){
            error /= 50;
        }
        if (errors[0] < 50 && errors[0] + 20 < errors[1]){
            return "nidoran-f";
        }
        if (errors[1] < 50 && errors[1] + 20 < errors[0]){
            return "nidoran-m";
        }
    }
    return "";
}


} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
