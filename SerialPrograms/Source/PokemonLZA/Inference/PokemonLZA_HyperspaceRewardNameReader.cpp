/*  Hyperspace Reward Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_RawOCR.h"
#include "PokemonLZA_HyperspaceRewardNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

HyperspaceRewardNameReader& HyperspaceRewardNameReader::instance(){
    static HyperspaceRewardNameReader reader;
    return reader;
}


HyperspaceRewardNameReader::HyperspaceRewardNameReader()
    : SmallDictionaryMatcher("PokemonSV/AAT/TournamentPrizeNameOCR.json") //TODO: Get actual reward list
{}

OCR::StringMatchResult HyperspaceRewardNameReader::read_substring(
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
}
