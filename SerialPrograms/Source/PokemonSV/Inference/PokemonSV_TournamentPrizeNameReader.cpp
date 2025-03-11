/*  Tournament Prize Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_RawOCR.h"
#include "PokemonSV_TournamentPrizeNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

TournamentPrizeNameReader& TournamentPrizeNameReader::instance(){
    static TournamentPrizeNameReader reader;
    return reader;
}


TournamentPrizeNameReader::TournamentPrizeNameReader()
    : SmallDictionaryMatcher("PokemonSV/AAT/TournamentPrizeNameOCR.json")
{}

OCR::StringMatchResult TournamentPrizeNameReader::read_substring(
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
