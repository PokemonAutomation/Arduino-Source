/*  Shop Item Price Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_Routines.h"
#include "PokemonBDSP_ShopItemPriceReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShopItemPriceReader::ShopItemPriceReader()
    : SmallDictionaryMatcher("PokemonBDSP/ShopItemPriceOCR.json")
{}

const ShopItemPriceReader& ShopItemPriceReader::instance(){
    static ShopItemPriceReader reader;
    return reader;
}

OCR::StringMatchResult ShopItemPriceReader::read_substring(
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
