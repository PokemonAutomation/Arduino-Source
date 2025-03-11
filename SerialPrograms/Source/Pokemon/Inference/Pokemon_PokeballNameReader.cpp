/*  Pokeball Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_RawOCR.h"
#include "Pokemon_PokeballNameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


PokeballNameReader& PokeballNameReader::instance(){
    static PokeballNameReader reader;
    return reader;
}


PokeballNameReader::PokeballNameReader()
    : SmallDictionaryMatcher("Pokemon/PokeballNameOCR.json")
{}

OCR::StringMatchResult PokeballNameReader::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        MAX_LOG10P, MAX_LOG10P_SPREAD, min_text_ratio, max_text_ratio
    );
}



}
}
