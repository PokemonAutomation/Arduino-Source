/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon_IVCheckerReader.h"

namespace PokemonAutomation{
namespace Pokemon{



const IVCheckerReader& IVCheckerReader::instance(){
    static IVCheckerReader reader;
    return reader;
}

IVCheckerReader::IVCheckerReader()
    : SmallDictionaryMatcher("Pokemon/IVCheckerOCR.json")
{}

OCR::StringMatchResult IVCheckerReader::read_substring(
    LoggerQt& logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        MAX_LOG10P, MAX_LOG10P_SPREAD,
        min_text_ratio, max_text_ratio
    );
}




}
}
