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

OCR::MatchResult PokeballNameReader::read_substring(
    Language language,
    const QImage& image
) const{
    QString text = OCR::ocr_read(language, image);
    return match_substring(language, text);
}
OCR::MatchResult PokeballNameReader::read_substring(
    Language language,
    const std::string& expected,
    const QImage& image
) const{
    QString text = OCR::ocr_read(language, image);
    return match_substring(language, expected, text);
}


}
}
