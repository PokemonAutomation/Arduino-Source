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

OCR::StringMatchResult PokemonNameReader::read_substring(
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

