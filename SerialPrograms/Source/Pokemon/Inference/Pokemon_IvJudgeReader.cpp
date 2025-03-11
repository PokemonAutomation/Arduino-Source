/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon_IvJudgeReader.h"

namespace PokemonAutomation{
namespace Pokemon{

namespace{

std::string iv_checker_value_to_string(IvJudgeValue value){
    const char* names[] = {
        "UnableToDetect",
        "NoGood",
        "Decent",
        "PrettyGood",
        "VeryGood",
        "Fantastic",
        "Best",
        "HyperTrained",
    };

    return names[int(value)];
}

}

IvJudgeReader::IvJudgeReader(const std::string& json_path)
    : SmallDictionaryMatcher(json_path)
{}

OCR::StringMatchResult IvJudgeReader::read_substring(
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



std::string IvJudgeReader::Results::to_string() const{
    return  "HP: " + iv_checker_value_to_string(hp)
         + ", Att: " + iv_checker_value_to_string(attack)
         + ", Def: " + iv_checker_value_to_string(defense)
         + ", S.Att: " + iv_checker_value_to_string(spatk)
         + ", S.Def: " + iv_checker_value_to_string(spdef)
         + ", Spd: " + iv_checker_value_to_string(speed);
}


}
}
