/*  Nature Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_Routines.h"
#include "Pokemon_NatureReader.h"

namespace PokemonAutomation{
namespace Pokemon{

NatureReader::NatureReader(const std::string& json_path)
    : SmallDictionaryMatcher(json_path)
{}

OCR::StringMatchResult NatureReader::read_substring(
    Logger& logger,
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

std::string NatureReader::Results::to_string() const{
    return "Nature: " + NATURE_CHECKER_VALUE_STRINGS().get_string(nature);
}


}
}
