/*  Stats Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonFRLG_StatsReader.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/ImageManip.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Pokemon/Inference/Pokemon_NatureReader.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_DigitReader.h"
#include <opencv2/imgproc.hpp>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {

StatsReader::StatsReader(Color color)
        : m_color(color), m_box_nature(0.028976, 0.729610, 0.502487, 0.066639),
            m_box_level(0.052000, 0.120140, 0.099000, 0.069416),
            m_box_name(0.163158, 0.122917, 0.262811, 0.066639),
            m_box_gender(0.430769, 0.114423, 0.034615, 0.081731),
            m_box_hp(0.805274, 0.131247, 0.183790, 0.066639),
            m_box_attack(0.891000, 0.245089, 0.097607, 0.066639),
            m_box_defense(0.891000, 0.325612, 0.097607, 0.066639),
            m_box_sp_attack(0.891000, 0.406134, 0.097607, 0.066639),
            m_box_sp_defense(0.891000, 0.486657, 0.097607, 0.066639),
            m_box_speed(0.891000, 0.567180, 0.097607, 0.066639){}

void StatsReader::make_overlays(VideoOverlaySet &items) const {
    const BoxOption &GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_nature));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_level));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_name));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_hp));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_attack));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_defense));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_sp_attack));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_sp_defense));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_speed));
}

void StatsReader::read_page1(
    Logger &logger, Language language,
    const ImageViewRGB32 &frame,
    PokemonFRLG_Stats &stats,
    const std::set<std::string>& subset
){
    const bool save_debug_images = GlobalSettings::instance().SAVE_DEBUG_IMAGES;
    ImageViewRGB32 game_screen =
            extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    // Read Name (white text on lilac background).
    // Use multifiltered OCR across multiple narrow white bands. This tolerates
    // brightness shifts (down to ~0xc0) while still preferring cleaner bands.
    const std::vector<OCR::TextColorRange> name_text_color_ranges{
        {combine_rgb(224, 224, 224), combine_rgb(255, 255, 255)},
        {combine_rgb(208, 208, 208), combine_rgb(255, 255, 255)},
        {combine_rgb(192, 192, 192), combine_rgb(255, 255, 255)},
    };

    if (subset.size() > 0){
        auto name_result = Pokemon::PokemonNameReader(subset).read_substring(
                logger, language, extract_box_reference(game_screen, m_box_name),
                name_text_color_ranges);
        if (!name_result.results.empty()){
            stats.name = name_result.results.begin()->second.token;
        }
    }else{
        auto name_result = Pokemon::PokemonNameReader::instance().read_substring(
                logger, language, extract_box_reference(game_screen, m_box_name),
                name_text_color_ranges);
        if (!name_result.results.empty()){
            stats.name = name_result.results.begin()->second.token;
        }
    }

    // Detect gender by comparing red vs blue pixels
    ImageViewRGB32 gender_box = extract_box_reference(game_screen, m_box_gender);

    const bool replace_color_within_range = false;
    const ImageRGB32 red_region = filter_rgb32_range(
        gender_box,
        combine_rgb(150, 0, 0), combine_rgb(255, 100, 100), Color(0), replace_color_within_range
    );
    const size_t num_red_pixels = image_stats(red_region).count;

    const ImageRGB32 blue_region = filter_rgb32_range(
        gender_box,
        combine_rgb(0, 0, 180), combine_rgb(130, 130, 255), Color(0), replace_color_within_range
    );
    const size_t num_blue_pixels = image_stats(blue_region).count;

    const double threshold = gender_box.width() * gender_box.height() * 0.1;

    if (num_red_pixels > threshold){
        stats.gender = SummaryGender::Female;
    }else if (num_blue_pixels > threshold){
        stats.gender = SummaryGender::Male;
    } else {
        stats.gender = SummaryGender::Genderless;
    }



    ImageViewRGB32 level_box = extract_box_reference(game_screen, m_box_level);

    ImageRGB32 level_upscaled =
            level_box.scale_to(level_box.width() * 4, level_box.height() * 4);
    if (save_debug_images){
        level_upscaled.save("DebugDumps/ocr_level_upscaled.png");
    }

    // The level has a colored (lilac) background. The text is white, with a
    // gray/black shadow. To bridge the gaps and make a solid black character on a
    // white background: We want to turn BOTH the bright white text AND the dark
    // shadow into BLACK pixels, and turn the mid-tone lilac background into
    // WHITE. We can do this by keeping pixels that are very bright (text) or very
    // dark (shadow).

    ImageRGB32 level_ready(level_upscaled.width(), level_upscaled.height());
    for (size_t r = 0; r < level_upscaled.height(); r++){
        for (size_t c = 0; c < level_upscaled.width(); c++){
            Color pixel(level_upscaled.pixel(c, r));
            // If it's very bright (white text) OR very dark (shadow), it becomes
            // black text. Otherwise (lilac background), it becomes white background.
            if ((pixel.red() > 200 && pixel.green() > 200 && pixel.blue() > 200) ||
                    (pixel.red() < 100 && pixel.green() < 100 && pixel.blue() < 100)){
                level_ready.pixel(c, r) = (uint32_t)0xff000000; // Black
            }else{
                level_ready.pixel(c, r) = (uint32_t)0xffffffff; // White
            }
        }
    }

    if (save_debug_images){
        level_ready.save("DebugDumps/ocr_level_ready.png");
    }

    if (!GlobalSettings::instance().USE_PADDLE_OCR){
        // The level uses white text with dark shadow on a lilac background.
        // The digit reader's binarizer captures dark pixels (<=190 on all channels)
        // but NOT the white text (all channels 255 -> excluded). This leaves the
        // shadow outline fragmented into many small disconnected blobs.
        // Preprocess: convert bright-white text pixels to black so the binarizer
        // merges text + shadow into one solid connected blob per digit.
        ImageRGB32 preprocessed = filter_rgb32_range(
            level_box, 0xffc8c8c8, 0xffffffff, Color(0xff000000), true
        );
        if (save_debug_images){
            preprocessed.save("DebugDumps/ocr_level_preprocessed.png");
        }
        // Trim left 7% to exclude the "L" glyph blob (always at x~0).
        // The actual level digits start at ~13%+ of the box width.
        size_t lv_skip = preprocessed.width() * 7 / 100;
        ImagePixelBox digits_bbox(
            lv_skip, 0, preprocessed.width(),
            preprocessed.height()
        );
        ImageViewRGB32 level_digit_view =
                extract_box_reference(preprocessed, digits_bbox);
        if (save_debug_images){
            level_digit_view.save("DebugDumps/ocr_level_digits_trimmed.png");
        }
        // Use threshold 230 (not 175): lilac-background blob crops inherently
        // give higher RMSD than yellow stat-box crops due to background colour.
        stats.level = read_digits_waterfill_template(
                logger, level_digit_view, 230.0, DigitTemplateType::LevelBox,
                "levelDigit", 0x7F);
    }else{
        // Pass the binarized image to PaddleOCR
        stats.level = OCR::read_number(logger, level_ready, language);
    }

    // Read Nature (black text on white/beige).
    // Pipeline: BW -> invert -> morph close -> invert -> upscale -> smooth -> pad.
    // Morph close on the inverted image (text=white) bridges gaps in text
    // regions by growing white->eroding back. Works per-channel on CV_8UC4.
    const static Pokemon::NatureReader reader("Pokemon/NatureCheckerOCR.json");
    ImageViewRGB32 nature_raw = extract_box_reference(game_screen, m_box_nature);
    if (save_debug_images){
        nature_raw.save("DebugDumps/ocr_nature_0_raw.png");
    }

    // Step 1: BW at native resolution. Dark text [0..150] -> black.
    ImageRGB32 nature_bw = to_blackwhite_rgb32_range(
            nature_raw, true,
            combine_rgb(0, 0, 0), combine_rgb(150, 150, 150));
    if (save_debug_images){
        nature_bw.save("DebugDumps/ocr_nature_1_bw.png");
    }

    // Step 2: Invert -> MORPH_CLOSE -> Invert to bridge gaps.
    // On the inverted image, text is bright (255) and bg is dark (0).
    // MORPH_CLOSE (dilate then erode) fills small dark holes within
    // the bright text regions - exactly the 1px gaps we need to bridge.
    // A 3x3 kernel bridges 1px gaps. Two iterations bridges 2px gaps.
    {
        cv::Mat bw_mat = nature_bw.to_opencv_Mat();
        cv::Mat inverted;
        cv::bitwise_not(bw_mat, inverted);

        cv::Mat kernel = cv::getStructuringElement(
                cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::Mat closed;
        cv::morphologyEx(inverted, closed, cv::MORPH_CLOSE, kernel,
                cv::Point(-1, -1), 2);  // 2 iterations for 2px gaps

        cv::Mat result;
        cv::bitwise_not(closed, result);

        ImageRGB32 nature_filled(result.cols, result.rows);
        result.copyTo(nature_filled.to_opencv_Mat());
        nature_bw = std::move(nature_filled);
    }
    if (save_debug_images){
        nature_bw.save("DebugDumps/ocr_nature_2_gapfilled.png");
    }

    // Step 3: 4x bilinear upscale.
    int scale = 4;
    cv::Mat filled_mat = nature_bw.to_opencv_Mat();
    cv::Mat upscaled;
    cv::resize(filled_mat, upscaled,
            cv::Size(filled_mat.cols * scale, filled_mat.rows * scale),
            0, 0, cv::INTER_LINEAR);
    ImageRGB32 nature_up(upscaled.cols, upscaled.rows);
    upscaled.copyTo(nature_up.to_opencv_Mat());
    if (save_debug_images){
        nature_up.save("DebugDumps/ocr_nature_3_upscaled.png");
    }

    // Step 4: Smooth + re-threshold (same as preprocess_for_ocr).
    cv::Mat smoothed;
    cv::GaussianBlur(upscaled, smoothed, cv::Size(7, 7), 2.0);
    ImageRGB32 smoothed_img(smoothed.cols, smoothed.rows);
    smoothed.copyTo(smoothed_img.to_opencv_Mat());
    ImageRGB32 nature_smooth = to_blackwhite_rgb32_range(
            smoothed_img, true,
            combine_rgb(0, 0, 0), combine_rgb(128, 128, 128));
    if (save_debug_images){
        nature_smooth.save("DebugDumps/ocr_nature_4_smooth.png");
    }

    // Step 5: Pad with white border.
    ImageRGB32 nature_padded = pad_image(
            nature_smooth, nature_smooth.height() / 2, 0xffffffff);
    if (save_debug_images){
        nature_padded.save("DebugDumps/ocr_nature_5_padded.png");
    }

    // OCR left/right single-word crops and pick the best score.
    // This handles both "RASH nature." and "Nature DOCILE." while avoiding
    // noisy full-line matches. Fall back to full-line only if both halves fail.
    OCR::StringMatchResult best_nature_result;
    bool have_best_nature_result = false;

    auto consider_nature_result = [&](const OCR::StringMatchResult& result){
        if (result.results.empty()){
            return;
        }
        if (!have_best_nature_result
                || result.results.begin()->first < best_nature_result.results.begin()->first){
            best_nature_result = result;
            have_best_nature_result = true;
        }
    };

        // Left and right single-word attempts (silent - log final selection only).
    const ImageFloatBox left_word_box(0.00, 0.00, 0.56, 1.00);
    const ImageFloatBox right_word_box(0.44, 0.00, 0.56, 1.00);

    ImageViewRGB32 nature_left = extract_box_reference(nature_padded, left_word_box);
    ImageViewRGB32 nature_right = extract_box_reference(nature_padded, right_word_box);
    if (save_debug_images){
        nature_left.save("DebugDumps/ocr_nature_6_left_word.png");
        nature_right.save("DebugDumps/ocr_nature_7_right_word.png");
    }

        OCR::StringMatchResult left_result = reader.match_substring_from_image(
            nullptr, language, nature_left,
            Pokemon::NatureReader::MAX_LOG10P,
            Pokemon::NatureReader::MAX_LOG10P_SPREAD,
            OCR::PageSegMode::SINGLE_WORD);
        consider_nature_result(left_result);

        OCR::StringMatchResult right_result = reader.match_substring_from_image(
            nullptr, language, nature_right,
            Pokemon::NatureReader::MAX_LOG10P,
            Pokemon::NatureReader::MAX_LOG10P_SPREAD,
            OCR::PageSegMode::SINGLE_WORD);
        consider_nature_result(right_result);

        // Fallback: if both halves fail thresholding, try full-line once.
        if (!have_best_nature_result){
        OCR::StringMatchResult full_result = reader.match_substring_from_image(
            nullptr, language, nature_padded,
            Pokemon::NatureReader::MAX_LOG10P,
            Pokemon::NatureReader::MAX_LOG10P_SPREAD,
            OCR::PageSegMode::SINGLE_LINE);
        consider_nature_result(full_result);
        }

    if (have_best_nature_result){
        best_nature_result.log(logger, Pokemon::NatureReader::MAX_LOG10P, "Nature Final");
        stats.nature = best_nature_result.results.begin()->second.token;
    }
}

void StatsReader::read_page2(
    Logger &logger, const ImageViewRGB32 &frame,
    PokemonFRLG_Stats &stats
){
    ImageViewRGB32 game_screen =
            extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    auto read_stat = [&](const ImageFloatBox &box, const std::string &name){
        ImageViewRGB32 stat_region = extract_box_reference(game_screen, box);

        if (!GlobalSettings::instance().USE_PADDLE_OCR){
            // Tesseract-free path: waterfill segmentation + template matching
            // against the PokemonFRLG/Digits/0-9.png templates.
            return read_digits_waterfill_template(logger, stat_region);
        }

        // PaddleOCR path (original): preprocess then per-digit waterfill OCR.
        // Dark text [0..190] -> black. Threshold at 190 captures the
        // blurred gap pixels between segments, making bridges thicker.
        // Not higher than 190 to avoid capturing yellow bg edge noise.
        ImageRGB32 ocr_ready = preprocess_for_ocr(
            stat_region, name, 7, 2, true,
            combine_rgb(0, 0, 0), combine_rgb(190, 190, 190)
        );

        // Waterfill isolates each digit -> per-char SINGLE_CHAR OCR.
        return OCR::read_number_waterfill(
            logger, ocr_ready, 0xff000000,
            0xff808080
        );
    };

    // HP box: shift right 55% to clear the "/" character.
    ImageFloatBox total_hp_box(
        m_box_hp.x + m_box_hp.width * 0.60, m_box_hp.y,
        m_box_hp.width * 0.40, m_box_hp.height
    );

    auto assign_stat = [](std::optional<unsigned>& field, int value){
        if (value != -1){
            field = static_cast<unsigned>(value);
        }
    };
    assign_stat(stats.hp, read_stat(total_hp_box, "hp"));
    assign_stat(stats.attack, read_stat(m_box_attack, "attack"));
    assign_stat(stats.defense, read_stat(m_box_defense, "defense"));
    assign_stat(stats.sp_attack, read_stat(m_box_sp_attack, "spatk"));
    assign_stat(stats.sp_defense, read_stat(m_box_sp_defense, "spdef"));
    assign_stat(stats.speed, read_stat(m_box_speed, "speed"));
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

