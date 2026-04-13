/*  Trainer ID Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonFRLG_TrainerIdReader.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/ImageManip.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_DigitReader.h"
#include <opencv2/imgproc.hpp>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {

// Debug counter for unique filenames
static int debug_counter = 0;

// Full OCR preprocessing pipeline for GBA pixel fonts.
//
// GBA fonts are seven-segment-like with 1-pixel gaps between segments.
// Pipeline: blur at native -> smooth upscale -> BW -> smooth BW -> re-BW -> pad
//
// The native blur connects gaps. Post-BW padding provides margins.
static ImageRGB32 preprocess_for_ocr(
    const ImageViewRGB32 &image,
    int blur_kernel_size, int blur_passes,
    bool in_range_black, uint32_t bw_min,
    uint32_t bw_max
){
    const bool save_debug_images = GlobalSettings::instance().SAVE_DEBUG_IMAGES;
    int id = debug_counter++;
    std::string prefix = "DebugDumps/ocr_tid_" + std::to_string(id);

    // Save raw input
    if (save_debug_images){
        image.save(prefix + "_0_raw.png");
    }

    cv::Mat src = image.to_opencv_Mat();

    // Step 1: Gaussian blur at NATIVE resolution with 5x5 kernel.
    cv::Mat blurred_native;
    src.copyTo(blurred_native);
    if (blur_kernel_size > 0 && blur_passes > 0){
        for (int i = 0; i < blur_passes; i++){
            cv::GaussianBlur(
                blurred_native, blurred_native,
                cv::Size(blur_kernel_size, blur_kernel_size), 1.5
            );
        }
    }

    // Save blurred at native res
    ImageRGB32 blurred_native_img(blurred_native.cols, blurred_native.rows);
    blurred_native.copyTo(blurred_native_img.to_opencv_Mat());
    if (save_debug_images){
        blurred_native_img.save(prefix + "_1_blurred_native.png");
    }

    // Step 2: Smooth upscale 4x with bilinear interpolation.
    int scale_factor = 4;
    int new_w = static_cast<int>(image.width()) * scale_factor;
    int new_h = static_cast<int>(image.height()) * scale_factor;
    cv::Mat resized;
    cv::resize(
        blurred_native, resized, cv::Size(new_w, new_h), 0, 0,
        cv::INTER_LINEAR
    );

    // Save upscaled
    ImageRGB32 resized_img(resized.cols, resized.rows);
    resized.copyTo(resized_img.to_opencv_Mat());
    if (save_debug_images){
        resized_img.save(prefix + "_2_upscaled.png");
    }

    // Step 3: BW threshold on the smooth upscaled image.
    ImageRGB32 bw =
            to_blackwhite_rgb32_range(resized_img, in_range_black, bw_min, bw_max);
    if (save_debug_images){
        bw.save(prefix + "_3_bw.png");
    }

    // Step 4: Post-BW smoothing -> re-threshold.
    cv::Mat bw_mat = bw.to_opencv_Mat();
    cv::Mat smoothed;
    cv::GaussianBlur(bw_mat, smoothed, cv::Size(7, 7), 2.0);

    // Re-threshold: convert smoothed back to ImageRGB32 and BW threshold.
    ImageRGB32 smoothed_img(smoothed.cols, smoothed.rows);
    smoothed.copyTo(smoothed_img.to_opencv_Mat());
    ImageRGB32 smooth_bw = to_blackwhite_rgb32_range(
            smoothed_img, true, combine_rgb(0, 0, 0), combine_rgb(128, 128, 128));
    if (save_debug_images){
        smooth_bw.save(prefix + "_4_smooth_bw.png");
    }

    // Step 5: Pad with white border (Tesseract needs margins).
    ImageRGB32 padded = pad_image(smooth_bw, smooth_bw.height() / 2, 0xffffffff);
    if (save_debug_images){
        padded.save(prefix + "_5_padded.png");
    }

    return padded;
}

TrainerIdReader::TrainerIdReader(Color color)
    : m_color(color)
    , m_box_tid(0.742683, 0.117314, 0.129734, 0.076006)
{}

void TrainerIdReader::make_overlays(VideoOverlaySet &items) const {
    const BoxOption &GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_tid));
}

uint16_t TrainerIdReader::read_tid(
    Logger &logger, const ImageViewRGB32 &frame
){
    ImageViewRGB32 game_screen =
            extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    
    ImageViewRGB32 tid_region = extract_box_reference(game_screen, m_box_tid);

    if (!GlobalSettings::instance().USE_PADDLE_OCR){
        // Tesseract-free path: waterfill segmentation + template matching
        // against the PokemonFRLG/Digits/0-9.png templates.
        return uint16_t(read_digits_waterfill_template(logger, tid_region));
    }

    // PaddleOCR path (original): preprocess then per-digit waterfill OCR.
    ImageRGB32 ocr_ready = preprocess_for_ocr(
        tid_region, 7, 2, true,
        combine_rgb(0, 0, 0), combine_rgb(190, 190, 190)
    );

    // Waterfill isolates each digit -> per-char SINGLE_CHAR OCR.
    return uint16_t(OCR::read_number_waterfill(
        logger, ocr_ready, 0xff000000,
        0xff808080
    ));
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

