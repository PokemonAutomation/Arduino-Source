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
#include "Common/Cpp/Concurrency/ThreadPool.h"

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


// Try OCR with all the given color filters. still running OCR on each individual character
// Return the best majority candidate. return -1 if failed to read.
// width_max: if a character width is greater than this value (likely means two characters are touching, will cause bad read), skip this filter.
// min_digit_area: if a character has area (aka pixel count) smaller than this value (likely noise or punctuations), skip this character
// 
// prioritize_numeric_only_results: 
//  - if true: if OCR reads only numeric characters, the candidate gets 2 votes. If OCR reads non-numeric characters, the candidate gets only 1 vote.
//  - if false: all reads only get 1 vote
//
// log_line_index: adds an index prefix to the logging lines for logging purposes when calling this function in parallel.
int read_number_waterfill_multifilter(
    Logger& logger,
    ThreadPool& thread_pool,
    const ImageViewRGB32& image,
    std::vector<std::pair<uint32_t, uint32_t>> filters,
    bool text_inside_range = true,
    bool prioritize_numeric_only_results = true,
    size_t width_max = (size_t)-1,
    size_t min_digit_area = 20,
    int8_t log_line_index = -1
 );


}
}
#endif
