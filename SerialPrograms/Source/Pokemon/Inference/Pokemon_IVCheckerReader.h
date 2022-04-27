/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_IVCheckerReader_H
#define PokemonAutomation_Pokemon_IVCheckerReader_H

#include "CommonFramework/OCR/OCR_SmallDictionaryMatcher.h"
#include "Pokemon/Pokemon_IVChecker.h"

namespace PokemonAutomation{
namespace Pokemon{



class IVCheckerReader : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;

public:
    struct Results{
        IVCheckerValue hp       = IVCheckerValue::UnableToDetect;
        IVCheckerValue attack   = IVCheckerValue::UnableToDetect;
        IVCheckerValue defense  = IVCheckerValue::UnableToDetect;
        IVCheckerValue spatk    = IVCheckerValue::UnableToDetect;
        IVCheckerValue spdef    = IVCheckerValue::UnableToDetect;
        IVCheckerValue speed    = IVCheckerValue::UnableToDetect;
    };

    static const IVCheckerReader& instance();

    OCR::StringMatchResult read_substring(
        LoggerQt& logger,
        Language language,
        const ConstImageRef& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;

private:
    IVCheckerReader();
};



}
}
#endif
