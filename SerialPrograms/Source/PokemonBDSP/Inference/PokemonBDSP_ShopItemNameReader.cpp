/*  Shop Item Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_Routines.h"
#include "PokemonBDSP_ShopItemNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShopItemNameReader::ShopItemNameReader()
    : SmallDictionaryMatcher("PokemonBDSP/ShopItemNameOCR.json")
{}

const ShopItemNameReader& ShopItemNameReader::instance(){
    static ShopItemNameReader reader;
    return reader;
}

OCR::StringMatchResult ShopItemNameReader::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image,
        OCR::BLACK_TEXT_FILTERS(),
        MAX_LOG10P, MAX_LOG10P_SPREAD,
        min_text_ratio, max_text_ratio
    );
}


}
}
}
