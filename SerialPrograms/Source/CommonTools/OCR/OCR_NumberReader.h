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
// line_index: specifies the current number's row. for logging purposes, when multithreaded.
int read_number_waterfill(
    Logger& logger, const ImageViewRGB32& image,
    uint32_t rgb32_min, uint32_t rgb32_max,
    bool text_inside_range = true,
    int8_t line_index = -1
 );

// run OCR on each individual character in the string of numbers.
// return empty string if OCR fails
//
// text_inside_range: binary filter is applied to the image so that any pixels within the color range will be turned black, and everything else will be white
// width_max: return empty string if any character's width is greater than width_max (likely means that two characters are touching, and so are treated as one large character)
// check_empty_string: if set to true, return empty string (and stop evaluation) if any character returns an empty string from OCR
 std::string read_number_waterfill_no_normalization(
    Logger& logger, const ImageViewRGB32& image,
    uint32_t rgb32_min, uint32_t rgb32_max,
    bool text_inside_range = true,
    size_t width_max = (size_t)-1,
    bool check_empty_string = false
 );

// Try OCR with all the given color filters. still running OCR on each individual character
// Return the best majority candidate. return -1 if failed to read.
// 
// prioritize_numeric_only_results: 
//  - if true: if OCR reads only numeric characters, the candidate gets 2 votes. If OCR reads non-numeric characters, the candidate gets only 1 vote.
//  - if false: all reads only get 1 vote
//
// line_index: specifies the current number's row. for logging purposes, when multithreaded.
int read_number_waterfill_multifilter(
    Logger& logger, const ImageViewRGB32& image,
    std::vector<std::pair<uint32_t, uint32_t>> filters,
    size_t width_max = (size_t)-1,
    bool text_inside_range = true,
    bool prioritize_numeric_only_results = true,
    int8_t line_index = -1
 );


}
}
#endif
