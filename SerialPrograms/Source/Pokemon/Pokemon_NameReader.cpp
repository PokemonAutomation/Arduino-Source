/*  Pokemon Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/OCR/RawOCR.h"
#include "Pokemon_NameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


PokemonNameReader::PokemonNameReader()
    : LargeDictionaryMatcher("Pokemon/PokemonNameOCR/PokemonOCR-")
{}

OCR::MatchResult PokemonNameReader::read_exact(
    Language language,
    const QImage& image
) const{
    QString text = OCR::ocr_read(language, image);
    return match_substring(language, text);
}
OCR::MatchResult PokemonNameReader::read_exact(
    Language language,
    const std::string& expected,
    const QImage& image
) const{
    QString text = OCR::ocr_read(language, image);
    return match_substring(language, expected, text);
}


}
}

