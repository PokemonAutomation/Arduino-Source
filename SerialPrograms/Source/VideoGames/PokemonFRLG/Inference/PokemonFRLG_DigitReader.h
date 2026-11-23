/*  FRLG Digit Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Reads a string of decimal digits from a stat region using waterfill
 *  segmentation on a blurred image to locate individual digit bounding boxes,
 *  then template-matches each cropped digit against the pre-stored digit
 *  templates (Resources/PokemonFRLG/Digits/0-9.png) on the unblurred original.
 *
 *  This is the Tesseract (PaddleOCR-free) fallback path.
 */

#ifndef PokemonAutomation_PokemonFRLG_DigitReader_H
#define PokemonAutomation_PokemonFRLG_DigitReader_H

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
    DialogBox,    // White dialog box: PokemonFRLG/DialogDigits/
};

// Read a number written in the FRLG pixel font, e.g. a stat, a level or a trainer ID.
// It finds each digit's shape, then compares the digit against 10 template images.
//
// template_type      The template set, chosen by the type of the UI the number is on (see
//                    `DigitTemplateType`).
// rmsd_threshold     The highest RMSD that still counts as a match. 175 by default;
//                    `StatsReader::read_level()` uses 230, because the lilac
//                    background raises the RMSD.
// dump_prefix        File name prefix of the digit crops saved to DebugDumps/.
// save_debug_images  Save each digit crop to DebugDumps/.
//
// Returns the number, or -1 if `stat_region` is empty or no digit can be found.
int read_digits_waterfill_template(
    Logger& logger,
    const ImageViewRGB32& stat_region,
    DigitTemplateType template_type = DigitTemplateType::StatBox,
    double rmsd_threshold = 175.0,
    const std::string& dump_prefix = "digit",
    bool save_debug_images = false
);


} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

#endif

