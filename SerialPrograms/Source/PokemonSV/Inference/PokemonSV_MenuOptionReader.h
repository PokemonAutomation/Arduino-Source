/*  Menu Option Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MenuOptionReader_H
#define PokemonAutomation_PokemonSV_MenuOptionReader_H

#include "CommonFramework/Language.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class MenuOptionReader : public OCR::SmallDictionaryMatcher{
    static constexpr double MAX_LOG10P = -1.30;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    MenuOptionReader();

    static MenuOptionReader& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;
};    



}
}
}
#endif
