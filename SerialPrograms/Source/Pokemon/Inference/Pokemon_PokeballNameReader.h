/*  Pokeball Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokeballNameReader_H
#define PokemonAutomation_Pokemon_PokeballNameReader_H

#include "CommonFramework/OCR/OCR_SmallDictionaryMatcher.h"

namespace PokemonAutomation{
namespace Pokemon{


class PokeballNameReader : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    PokeballNameReader();

    static PokeballNameReader& instance();

    OCR::StringMatchResult read_substring(
        LoggerQt& logger,
        Language language,
        const QImage& image
    ) const;

    OCR::StringMatchResult read_substring(
        LoggerQt& logger,
        Language language,
        const ConstImageRef& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;

};


}
}
#endif
