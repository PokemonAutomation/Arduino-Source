/*  Pokemon Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameReader_H
#define PokemonAutomation_Pokemon_PokemonNameReader_H

#include "CommonTools/OCR/OCR_LargeDictionaryMatcher.h"

namespace PokemonAutomation{
namespace Pokemon{


class PokemonNameReader : public OCR::LargeDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

private:
    PokemonNameReader();
public:
    PokemonNameReader(const std::set<std::string>& subset);

public:
    static const PokemonNameReader& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50,
        double max_log10p = MAX_LOG10P
    ) const;

};


}
}
#endif
