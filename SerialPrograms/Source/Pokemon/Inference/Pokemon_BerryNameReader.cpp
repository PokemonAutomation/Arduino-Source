/*  Berry Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_Routines.h"
#include "Pokemon_BerryNameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


BerryNameReader& BerryNameReader::instance(){
    static BerryNameReader reader;
    return reader;
}


BerryNameReader::BerryNameReader()
    : SmallDictionaryMatcher("Pokemon/BerryNameOCR.json")
{}

OCR::StringMatchResult BerryNameReader::read_substring(
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
