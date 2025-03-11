/*  Auction Item Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_RawOCR.h"
#include "PokemonSV_AuctionItemNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


AuctionItemNameReader& AuctionItemNameReader::instance(){
    static AuctionItemNameReader reader;
    return reader;
}


AuctionItemNameReader::AuctionItemNameReader()
    : SmallDictionaryMatcher("PokemonSV/Auction/AuctionItemNameOCR.json")
{}

OCR::StringMatchResult AuctionItemNameReader::read_substring(
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
