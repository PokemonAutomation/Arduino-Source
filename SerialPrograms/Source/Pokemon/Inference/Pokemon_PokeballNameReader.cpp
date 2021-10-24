/*  Pokeball Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/OCR/RawOCR.h"
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
    Logger& logger,
    Language language,
    const QImage& image
) const{
    QString text = OCR::ocr_read(language, image);
    OCR::StringMatchResult ret;
    match_substring(ret, language, text);
    ret.log(logger, MAX_LOG10P);
    ret.clear_beyond_log10p(MAX_LOG10P);
    return ret;
}


}
}
