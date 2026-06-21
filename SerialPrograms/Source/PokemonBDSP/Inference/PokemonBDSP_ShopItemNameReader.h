/*  Shop Item Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShopItemNameReader_H
#define PokemonAutomation_PokemonBDSP_ShopItemNameReader_H

#include "CommonFramework/Language.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ShopItemNameReader : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P        = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    ShopItemNameReader();

    static const ShopItemNameReader& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;
};


}
}
}
#endif
