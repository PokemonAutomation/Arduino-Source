/*  OCR Number Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_NumberReader_H
#define PokemonAutomation_CommonTools_OCR_NumberReader_H

#include <stdint.h>
#include <vector>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
namespace OCR{


//  Returns -1 if no number is found.
//  No processing is done on the image. It is OCR'ed directly.
//  The best way to use read_number() is to not give Language as a parameter.
//  This is because the default language is English, since it works best for OCR for numbers.
int read_number(Logger& logger, const ImageViewRGB32& image, Language language = Language::English);


//  This version attempts to improve reliability by first isolating each number
//  via waterfill. Then it OCRs each number by itself and recombines them at the
//  end. This requires specifying the color range for the text.
//
// text_inside_range: binary filter is applied to the image so that any pixels within the color range will be turned black, and everything else will be white
// width_max: return -1 if any character's width is greater than width_max (likely means that two characters are touching, and so are treated as one large character)
// check_empty_string: if set to true, return -1 (and stop evaluation) if any character returns an empty string from OCR
int read_number_waterfill(
    Logger& logger, const ImageViewRGB32& image,
    uint32_t rgb32_min, uint32_t rgb32_max,
    bool text_inside_range = true,
    uint32_t width_max = UINT32_MAX,
    bool check_empty_string = false
 );

// Try OCR with all the given color filters.
// Return the best majority candidate
int read_number_waterfill(
    Logger& logger, const ImageViewRGB32& image,
    std::vector<std::pair<uint32_t, uint32_t>> filters,
    uint32_t width_max,
    bool text_inside_range = true
 );


}
}
#endif
