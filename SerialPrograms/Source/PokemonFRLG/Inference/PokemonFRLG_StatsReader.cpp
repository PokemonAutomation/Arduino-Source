/*  Stats Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonFRLG_StatsReader.h"
#include "PokemonFRLG_DigitReader.h"
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
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Pokemon/Inference/Pokemon_NatureReader.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include <opencv2/imgproc.hpp>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {

// Debug counter for unique filenames
static int debug_counter = 0;

// Full OCR preprocessing pipeline for GBA pixel fonts.
//
// GBA fonts are seven-segment-like with 1-pixel gaps between segments.
// Pipeline: blur at native → smooth upscale → BW → smooth BW → re-BW → pad
//
// The native blur connects gaps. Post-BW padding provides margins.
static ImageRGB32 preprocess_for_ocr(const ImageViewRGB32 &image,
                                     const std::string &label,
                                     int blur_kernel_size, int blur_passes,
                                     bool in_range_black, uint32_t bw_min,
                                     uint32_t bw_max) {
  int id = debug_counter++;
  std::string prefix = "DebugDumps/ocr_" + label + "_" + std::to_string(id);

  // Save raw input
  image.save(prefix + "_0_raw.png");

  cv::Mat src = image.to_opencv_Mat();

  // Step 1: Gaussian blur at NATIVE resolution with 5×5 kernel.
  // The 5×5 kernel reaches 2 pixels away (vs 1px for 3×3), bridging
  // wider gaps in the seven-segment font. Two passes for heavy smoothing.
  cv::Mat blurred_native;
  src.copyTo(blurred_native);
  if (blur_kernel_size > 0 && blur_passes > 0) {
    for (int i = 0; i < blur_passes; i++) {
      cv::GaussianBlur(blurred_native, blurred_native,
                       cv::Size(blur_kernel_size, blur_kernel_size), 1.5);
    }
  }

  // Save blurred at native res
  ImageRGB32 blurred_native_img(blurred_native.cols, blurred_native.rows);
  blurred_native.copyTo(blurred_native_img.to_opencv_Mat());
  blurred_native_img.save(prefix + "_1_blurred_native.png");

  // Step 2: Smooth upscale 4× with bilinear interpolation.
  int scale_factor = 4;
  int new_w = static_cast<int>(image.width()) * scale_factor;
  int new_h = static_cast<int>(image.height()) * scale_factor;
  cv::Mat resized;
  cv::resize(blurred_native, resized, cv::Size(new_w, new_h), 0, 0,
             cv::INTER_LINEAR);

  // Save upscaled
  ImageRGB32 resized_img(resized.cols, resized.rows);
  resized.copyTo(resized_img.to_opencv_Mat());
  resized_img.save(prefix + "_2_upscaled.png");

  // Step 3: BW threshold on the smooth upscaled image.
  ImageRGB32 bw =
      to_blackwhite_rgb32_range(resized_img, in_range_black, bw_min, bw_max);
  bw.save(prefix + "_3_bw.png");

  // Step 4: Post-BW smoothing → re-threshold.
  // The BW image has angular seven-segment shapes. GaussianBlur on the
  // binary image creates gray anti-aliased edges. Re-thresholding at 128
  // rounds the corners into natural smooth digit shapes that Tesseract
  // recognizes much better. This is equivalent to morphological closing.
  cv::Mat bw_mat = bw.to_opencv_Mat();
  cv::Mat smoothed;
  cv::GaussianBlur(bw_mat, smoothed, cv::Size(7, 7), 2.0);

  // Re-threshold: convert smoothed back to ImageRGB32 and BW threshold.
  // After blur on BW: text areas are dark gray (~0-64), bg areas are
  // light gray (~192-255), edge zones are mid-gray (~64-192).
  // Threshold at [0..128] captures text + expanded edges → BLACK.
  ImageRGB32 smoothed_img(smoothed.cols, smoothed.rows);
  smoothed.copyTo(smoothed_img.to_opencv_Mat());
  ImageRGB32 smooth_bw = to_blackwhite_rgb32_range(
      smoothed_img, true, combine_rgb(0, 0, 0), combine_rgb(128, 128, 128));
  smooth_bw.save(prefix + "_4_smooth_bw.png");

  // Step 5: Pad with white border (Tesseract needs margins).
  ImageRGB32 padded = pad_image(smooth_bw, smooth_bw.height() / 2, 0xffffffff);
  padded.save(prefix + "_5_padded.png");

  return padded;
}

StatsReader::StatsReader(Color color)
    : m_color(color), m_box_nature(0.028976, 0.729610, 0.502487, 0.065251),
      m_box_level(0.052000, 0.120140, 0.099000, 0.069416),
      m_box_name(0.163158, 0.122917, 0.262811, 0.066639),
      m_box_hp(0.815558, 0.131247, 0.173049, 0.065251),
      m_box_attack(0.891000, 0.245089, 0.097607, 0.063862),
      m_box_defense(0.891000, 0.325612, 0.097607, 0.066639),
      m_box_sp_attack(0.891000, 0.406134, 0.097607, 0.066639),
      m_box_sp_defense(0.891000, 0.486657, 0.097607, 0.063862),
      m_box_speed(0.891000, 0.567180, 0.097607, 0.066639) {}

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

void StatsReader::read_page1(Logger &logger, Language language,
                             const ImageViewRGB32 &frame,
                             PokemonFRLG_Stats &stats) {
  ImageViewRGB32 game_screen =
      extract_box_reference(frame, GameSettings::instance().GAME_BOX);

  // Read Name (white text with shadow)
  auto name_result = Pokemon::PokemonNameReader::instance().read_substring(
      logger, language, extract_box_reference(game_screen, m_box_name),
      {{combine_rgb(235, 235, 235), combine_rgb(255, 255, 255)}});
  if (!name_result.results.empty()) {
    stats.name = name_result.results.begin()->second.token;
  }

  ImageViewRGB32 level_box = extract_box_reference(game_screen, m_box_level);
  
  // As per user request, upscale the level box without additional blur/morphological filters.
  ImageRGB32 level_upscaled = level_box.scale_to(level_box.width() * 4, level_box.height() * 4);
  level_upscaled.save("DebugDumps/ocr_level_upscaled.png");
  
  // The level has a colored (lilac) background. The text is white, with a gray/black shadow.
  // To bridge the gaps and make a solid black character on a white background:
  // We want to turn BOTH the bright white text AND the dark shadow into BLACK pixels,
  // and turn the mid-tone lilac background into WHITE.
  // We can do this by keeping pixels that are very bright (text) or very dark (shadow).
  
  ImageRGB32 level_ready(level_upscaled.width(), level_upscaled.height());
  for (size_t r = 0; r < level_upscaled.height(); r++) {
      for (size_t c = 0; c < level_upscaled.width(); c++) {
          Color pixel(level_upscaled.pixel(c, r));
          // If it's very bright (white text) OR very dark (shadow), it becomes black text.
          // Otherwise (lilac background), it becomes white background.
          if ((pixel.red() > 200 && pixel.green() > 200 && pixel.blue() > 200) || 
              (pixel.red() < 100 && pixel.green() < 100 && pixel.blue() < 100)) {
              level_ready.pixel(c, r) = (uint32_t)0xff000000; // Black
          } else {
              level_ready.pixel(c, r) = (uint32_t)0xffffffff; // White
          }
      }
  }
  
  level_ready.save("DebugDumps/ocr_level_ready.png");

  if (!GlobalSettings::instance().USE_PADDLE_OCR) {
    // The level uses white text with dark shadow on a lilac background.
    // The digit reader's binarizer captures dark pixels (≤190 on all channels)
    // but NOT the white text (all channels 255 → excluded). This leaves the
    // shadow outline fragmented into many small disconnected blobs.
    // Preprocess: convert bright-white text pixels to black so the binarizer
    // merges text + shadow into one solid connected blob per digit.
    ImageRGB32 preprocessed(level_box.width(), level_box.height());
    for (size_t r = 0; r < level_box.height(); r++) {
      for (size_t c = 0; c < level_box.width(); c++) {
        Color px(level_box.pixel(c, r));
        if (px.red() > 200 && px.green() > 200 && px.blue() > 200) {
          preprocessed.pixel(c, r) = 0xff000000u; // white text → black
        } else {
          preprocessed.pixel(c, r) = level_box.pixel(c, r); // keep as-is
        }
      }
    }
    preprocessed.save("DebugDumps/ocr_level_preprocessed.png");
    stats.level = read_digits_waterfill_template(
        logger, preprocessed, 175.0,
        "PokemonFRLG/LevelDigits/", "levelDigit",
        0x7F  // tighter threshold: prevents blurred lilac (B~208→~156) from
              // being captured as foreground (190 threshold would capture it)
    );
  } else {
    // Pass the binarized image to PaddleOCR
    stats.level = OCR::read_number(logger, level_ready, language);
  }

  // Read Nature (black text on white/beige)
  const static Pokemon::NatureReader reader("Pokemon/NatureCheckerOCR.json");
  auto nature_result = reader.read_substring(
      logger, language, extract_box_reference(game_screen, m_box_nature),
      {{combine_rgb(0, 0, 0), combine_rgb(100, 100, 100)}});
  if (!nature_result.results.empty()) {
    stats.nature = nature_result.results.begin()->second.token;
  }
}

void StatsReader::read_page2(Logger &logger, const ImageViewRGB32 &frame,
                             PokemonFRLG_Stats &stats) {
  ImageViewRGB32 game_screen =
      extract_box_reference(frame, GameSettings::instance().GAME_BOX);

  auto read_stat = [&](const ImageFloatBox &box, const std::string &name) {
    ImageViewRGB32 stat_region = extract_box_reference(game_screen, box);

    if (!GlobalSettings::instance().USE_PADDLE_OCR) {
      // Tesseract-free path: waterfill segmentation + template matching
      // against the PokemonFRLG/Digits/0-9.png templates.
      return read_digits_waterfill_template(logger, stat_region);
    }

    // PaddleOCR path (original): preprocess then per-digit waterfill OCR.
    // Dark text [0..190] → black. Threshold at 190 captures the
    // blurred gap pixels between segments, making bridges thicker.
    // Not higher than 190 to avoid capturing yellow bg edge noise.
    ImageRGB32 ocr_ready =
        preprocess_for_ocr(stat_region, name, 7, 2, true, combine_rgb(0, 0, 0),
                           combine_rgb(190, 190, 190));

    // Waterfill isolates each digit → per-char SINGLE_CHAR OCR.
    return OCR::read_number_waterfill(logger, ocr_ready, 0xff000000,
                                      0xff808080);
  };

  // HP box: shift right 70% to clear the "/" character.
  ImageFloatBox total_hp_box(m_box_hp.x + m_box_hp.width * 0.7, m_box_hp.y,
                             m_box_hp.width * 0.3, m_box_hp.height);

  stats.hp = read_stat(total_hp_box, "hp");
  stats.attack = read_stat(m_box_attack, "attack");
  stats.defense = read_stat(m_box_defense, "defense");
  stats.sp_attack = read_stat(m_box_sp_attack, "spatk");
  stats.sp_defense = read_stat(m_box_sp_defense, "spdef");
  stats.speed = read_stat(m_box_speed, "speed");
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
