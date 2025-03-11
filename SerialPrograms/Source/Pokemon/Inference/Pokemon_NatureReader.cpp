/*  Nature Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_Routines.h"
#include "Pokemon_NatureReader.h"

namespace PokemonAutomation{
namespace Pokemon{

namespace{

std::string nature_checker_value_to_string(NatureCheckerValue value){
    const char* names[] = {
        "UnableToDetect",
        "Any",
        "Adamant",
        "Bashful",
        "Bold",
        "Brave",
        "Calm",
        "Careful",
        "Docile",
        "Gentle",
        "Hardy",
        "Hasty",
        "Impish",
        "Jolly",
        "Lax",
        "Lonely",
        "Mild",
        "Modest",
        "Naive",
        "Naughty",
        "Quiet",
        "Quirky",
        "Rash",
        "Relaxed",
        "Sassy",
        "Serious",
        "Timid",
        "Last"
    };

    return names[int(value)];
}
}

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
    return  "Nature: " + nature_checker_value_to_string(nature);
}


}
}
