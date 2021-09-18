/*  Pokemon Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/OCR/RawOCR.h"
#include "Pokemon_NameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


const PokemonNameReader& PokemonNameReader::instance(){
    static PokemonNameReader reader;
    return reader;
}


PokemonNameReader::PokemonNameReader()
    : LargeDictionaryMatcher("Pokemon/PokemonNameOCR/PokemonOCR-", nullptr, false)
{}
PokemonNameReader::PokemonNameReader(const std::set<std::string>& subset)
    : LargeDictionaryMatcher("Pokemon/PokemonNameOCR/PokemonOCR-", &subset, false)
{}

OCR::MatchResult PokemonNameReader::read_substring(
    Language language,
    const QImage& image
) const{
    QString text = OCR::ocr_read(language, image);
    return match_substring(language, text);
}
OCR::MatchResult PokemonNameReader::read_substring(
    Language language,
    const std::string& expected,
    const QImage& image
) const{
    QString text = OCR::ocr_read(language, image);
    return match_substring(language, expected, text);
}


}
}

