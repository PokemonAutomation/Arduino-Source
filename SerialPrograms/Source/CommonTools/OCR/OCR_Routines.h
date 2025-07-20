/*  OCR Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_Routines_H
#define PokemonAutomation_CommonTools_OCR_Routines_H

#include <cstdint>
#include <vector>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace OCR{

struct StringMatchResult;
class DictionaryMatcher;



struct TextColorRange{
    uint32_t mins;
    uint32_t maxs;

    TextColorRange(uint32_t p_mins, uint32_t p_maxs)
        : mins(p_mins)
        , maxs(p_maxs)
    {}
};


StringMatchResult multifiltered_OCR(
    Language language, const DictionaryMatcher& dictionary, const ImageViewRGB32& image,
    const std::vector<TextColorRange>& text_color_ranges,
    double log10p_spread,
    double min_text_ratio = 0.01, double max_text_ratio = 0.50
);


const std::vector<TextColorRange>& BLACK_TEXT_FILTERS();
const std::vector<TextColorRange>& WHITE_TEXT_FILTERS();
const std::vector<TextColorRange>& BLACK_OR_WHITE_TEXT_FILTERS();
const std::vector<TextColorRange>& BLUE_TEXT_FILTERS();



}
}
#endif
