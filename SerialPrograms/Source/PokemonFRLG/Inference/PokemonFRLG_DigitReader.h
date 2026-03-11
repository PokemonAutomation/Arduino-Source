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
#define PokemonAutomation_PokemonFRLG_DigitReader_H

#include <string>

namespace PokemonAutomation {
class Logger;
class ImageViewRGB32;

namespace NintendoSwitch {
namespace PokemonFRLG {

// Read a string of decimal digits from `stat_region`.
//
// template_subdir  Resource subdirectory containing 0-9.png templates.
//                  Defaults to PokemonFRLG/Digits/ (yellow stat boxes).
//                  Pass "PokemonFRLG/LevelDigits/" for the lilac level box.
// dump_prefix      Prefix used when saving debug crop PNGs to DebugDumps/.
//
// Returns the parsed integer, or -1 on failure.
int read_digits_waterfill_template(
    Logger& logger,
    const ImageViewRGB32& stat_region,
    double rmsd_threshold = 175.0,
    const std::string& template_subdir = "PokemonFRLG/Digits/",
    const std::string& dump_prefix = "digit",
    uint8_t binarize_high = 0xBE   // 0xBE=190 for yellow stat boxes;
                                   // use 0x7F=127 for lilac level box
);

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

#endif
