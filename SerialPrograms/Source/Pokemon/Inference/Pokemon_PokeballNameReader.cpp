/*  Pokeball Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "Pokemon_PokeballNameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


PokeballNameReader& PokeballNameReader::instance(){
    static PokeballNameReader reader;
    return reader;
}


PokeballNameReader::PokeballNameReader()
    : SmallDictionaryMatcher("Pokemon/PokeballNameOCR.json")
{}

OCR::StringMatchResult PokeballNameReader::read_substring(
    LoggerQt& logger,
    Language language,
    const QImage& image
) const{
    QString text = OCR::ocr_read(language, image);
    OCR::StringMatchResult ret = match_substring(language, text);
    ret.log(logger, MAX_LOG10P);
    ret.clear_beyond_log10p(MAX_LOG10P);
    return ret;
}
OCR::StringMatchResult PokeballNameReader::read_substring(
    LoggerQt& logger,
    Language language,
    const ConstImageRef& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio
) const{
    return match_substring_from_image_multifiltered(
        logger, language, image, text_color_ranges,
        MAX_LOG10P, 0.5, min_text_ratio, max_text_ratio
    );
}



}
}
