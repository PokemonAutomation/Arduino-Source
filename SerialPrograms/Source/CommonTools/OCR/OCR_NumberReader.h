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
int read_number_waterfill(
    Logger& logger, const ImageViewRGB32& image,
    uint32_t rgb32_min, uint32_t rgb32_max,
    bool text_inside_range = true
 );

// applies color filters on the text, until each individual waterfilled character/number is less than width_max. then apply OCR.
// this solves the problem where characters are too close to each other and touch, causing more than one character to be OCR'ed
// also, if running OCR on a character results in an empty string, try the next color filter
// the filters should be arranged in order of preference. (probably should be arranged broadest to narrowest)
int read_number_waterfill(
    Logger& logger, const ImageViewRGB32& image,
    std::vector<std::pair<uint32_t, uint32_t>> filters,
    uint32_t width_max,
    bool text_inside_range = true
 );


}
}
#endif
