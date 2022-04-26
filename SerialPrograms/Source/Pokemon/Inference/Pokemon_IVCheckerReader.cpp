/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon_IVCheckerReader.h"

namespace PokemonAutomation{
namespace Pokemon{



const IVCheckerReader& IVCheckerReader::instance(){
    static IVCheckerReader reader;
    return reader;
}

IVCheckerReader::IVCheckerReader()
    : SmallDictionaryMatcher("Pokemon/IVCheckerOCR.json")
{}

OCR::StringMatchResult IVCheckerReader::read_substring(
    LoggerQt& logger,
    Language language,
    const ConstImageRef& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
) const{
    OCR::StringMatchResult ret = OCR::multifiltered_OCR(
        language, *this, image,
        text_color_ranges,
        0.5, min_text_ratio, max_text_ratio
    );
    ret.log(logger, MAX_LOG10P);
    ret.clear_beyond_log10p(MAX_LOG10P);
    return ret;
}




}
}
