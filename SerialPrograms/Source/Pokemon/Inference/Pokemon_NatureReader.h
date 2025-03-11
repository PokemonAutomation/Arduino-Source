/*  Nature Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_NatureReader_H
#define PokemonAutomation_Pokemon_NatureReader_H

#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "Pokemon/Pokemon_NatureChecker.h"

namespace PokemonAutomation{
namespace Pokemon{

class NatureReader : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    struct Results{
        NatureCheckerValue nature = NatureCheckerValue::UnableToDetect;
        std::string to_string() const;
    };

    NatureReader(const std::string& json_path);

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
