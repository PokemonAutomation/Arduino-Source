/*  Location Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_Routines.h"
#include "PokemonLZA_LocationNameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


LocationNameReader& LocationNameReader::instance(){
    static LocationNameReader reader;
    return reader;
}


LocationNameReader::LocationNameReader()
    : SmallDictionaryMatcher("PokemonLZA/LocationName.json")
{}

OCR::StringMatchResult LocationNameReader::read_substring(
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
