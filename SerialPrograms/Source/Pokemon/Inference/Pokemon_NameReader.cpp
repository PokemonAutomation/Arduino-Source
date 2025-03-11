/*  Pokemon Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_RawOCR.h"
#include "Pokemon_NameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


const PokemonNameReader& PokemonNameReader::instance(){
    static PokemonNameReader reader;
    return reader;
}


PokemonNameReader::PokemonNameReader()
    : LargeDictionaryMatcher("Pokemon/PokemonNameOCR/PokemonOCR-", nullptr, false)
{}
PokemonNameReader::PokemonNameReader(const std::set<std::string>& subset)
    : LargeDictionaryMatcher("Pokemon/PokemonNameOCR/PokemonOCR-", &subset, false)
{}

OCR::StringMatchResult PokemonNameReader::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio,
    double max_log10p
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        max_log10p, MAX_LOG10P_SPREAD, min_text_ratio, max_text_ratio
    );
}


}
}

