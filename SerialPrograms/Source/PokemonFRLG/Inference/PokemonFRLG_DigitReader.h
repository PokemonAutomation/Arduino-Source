/*  FRLG Digit Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Reads a string of decimal digits from a stat region using waterfill
 *  segmentation on a blurred image to locate individual digit bounding boxes,
 *  then template-matches each cropped digit against the pre-stored digit
 *  templates (Resources/PokemonFRLG/Digits/0-9.png) on the unblurred original.
 *
 *  This is the Tesseract/PaddleOCR-free fallback path for USE_PADDLE_OCR=false.
 */

#ifndef PokemonAutomation_PokemonFRLG_DigitReader_H
#include <cstdint>
#include <string>

namespace PokemonAutomation{

class Logger;
class ImageViewRGB32;

namespace NintendoSwitch{
namespace PokemonFRLG{

enum class DigitTemplateType{
    StatBox,      // Yellow stat boxes (default): PokemonFRLG/Digits/
    LevelBox,     // Lilac level box: PokemonFRLG/LevelDigits/
};

// Full OCR preprocessing pipeline for GBA pixel fonts.
//
// GBA fonts are seven-segment-like with 1-pixel gaps between segments.
// Pipeline: blur at native -> smooth upscale -> BW -> smooth BW -> re-BW -> pad
//
// The native blur connects gaps. Post-BW padding provides margins.
ImageRGB32 preprocess_for_ocr(
    const ImageViewRGB32 &image,
    const std::string &label,
    int blur_kernel_size, int blur_passes,
    bool in_range_black, uint32_t bw_min,
    uint32_t bw_max
);

// Read a string of decimal digits from `stat_region`.
//
// template_type    Which template set to use (StatBox or LevelBox).
// dump_prefix      Prefix used when saving debug crop PNGs to DebugDumps/.
//
// Returns the parsed integer, or -1 on failure.
int read_digits_waterfill_template(
    Logger& logger,
    const ImageViewRGB32& stat_region,
    double rmsd_threshold = 175.0,
    DigitTemplateType template_type = DigitTemplateType::StatBox,
    const std::string& dump_prefix = "digit",
    uint8_t binarize_high = 0xBE   // 0xBE=190 for yellow stat boxes;
    // use 0x7F=127 for lilac level box
);

// Read a string of decimal digits from `stat_region` by splitting the region into
// a fixed number of equal-width segments, instead of using waterfill.
// Useful when digits are tightly packed.
//
// num_splits       The number of equal-width segments to split the region into.
// template_type    Which template set to use (StatBox or LevelBox).
// dump_prefix      Prefix used when saving debug crop PNGs to DebugDumps/.
//
// Returns the parsed integer, or -1 on failure.
int read_digits_fixed_width_template(
    Logger& logger,
    const ImageViewRGB32& stat_region,
    int num_splits = 2,
    double rmsd_threshold = 175.0,
    DigitTemplateType template_type = DigitTemplateType::LevelBox,
    const std::string& dump_prefix = "digit_split"
);

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

#endif

