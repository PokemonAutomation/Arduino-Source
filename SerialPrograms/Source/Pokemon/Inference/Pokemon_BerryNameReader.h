/*  Berry Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_BerryNameReader_H
#define PokemonAutomation_Pokemon_BerryNameReader_H

#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"

namespace PokemonAutomation{
namespace Pokemon{


class BerryNameReader : public OCR::SmallDictionaryMatcher{
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    BerryNameReader();

    static BerryNameReader& instance();

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
#endif
