/*  FRLG OCR Preprocessing
 *
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_PokemonFRLG_OcrPreprocessing_H
#define PokemonAutomation_PokemonFRLG_OcrPreprocessing_H

#include <string>
#include <vector>
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonTools/OCR/OCR_Routines.h"

namespace PokemonAutomation{

class ImageViewRGB32;

namespace NintendoSwitch{
namespace PokemonFRLG{


//  Gaussian-blur `image` so the gaps in the GBA font close up.
//  The blur is adjusted from the passed size to match the size of the image
ImageRGB32 preprocess_for_ocr(
    const ImageViewRGB32& image,
    int blur_kernel_size = 7, int blur_passes = 2
);

//  For dark GBA text against a light background
const std::vector<OCR::TextColorRange>& DARK_TEXT_FILTERS();

//  For white GBA text against a dark background
const std::vector<OCR::TextColorRange>& BRIGHT_TEXT_FILTERS();

// Resolve the monochrome gender suffix when OCR cannot distinguish Nidoran.
// Returns "nidoran-f", "nidoran-m", or "" if the suffix is unreadable.
std::string read_nidoran_symbol(
    const ImageViewRGB32& name, const std::vector<OCR::TextColorRange>& text_color_ranges
);


} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

#endif
