/*  Berry Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "Pokemon_BerryNameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


BerryNameReader& BerryNameReader::instance(){
    static BerryNameReader reader;
    return reader;
}


BerryNameReader::BerryNameReader()
    : SmallDictionaryMatcher("Pokemon/BerryNameOCR.json")
{}

OCR::StringMatchResult BerryNameReader::read_substring(
    LoggerQt& logger,
    Language language,
    const ConstImageRef& image
) const{
    QString text = OCR::ocr_read(language, image);
    OCR::StringMatchResult ret = match_substring(language, text);
    ret.log(logger, MAX_LOG10P);
    ret.clear_beyond_log10p(MAX_LOG10P);
    return ret;
}


}
}
