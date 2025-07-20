/*  Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "OCR_DictionaryMatcher.h"

namespace PokemonAutomation{
namespace OCR{


const DictionaryOCR& DictionaryMatcher::dictionary(Language language) const{
    auto iter = m_database.find(language);
    if (iter == m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Language not loaded.");
    }
    return iter->second;
}
DictionaryOCR& DictionaryMatcher::dictionary(Language language){
    ReadSpinLock lg(m_lock, "LargeDictionaryMatcher::dictionary()");
    auto iter = m_database.find(language);
    if (iter == m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Language not loaded.");
    }
    return iter->second;
}


StringMatchResult DictionaryMatcher::match_substring(
    Language language,
    const std::string& text,
    double log10p_spread
) const{
    return dictionary(language).match_substring(text, log10p_spread);
}

OCR::StringMatchResult DictionaryMatcher::match_substring_from_image_multifiltered(
    Logger* logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double max_log10p, double log10p_spread,
    double min_text_ratio, double max_text_ratio
) const{
    OCR::StringMatchResult ret = OCR::multifiltered_OCR(
        language, *this, image,
        text_color_ranges,
        log10p_spread, min_text_ratio, max_text_ratio
    );
    if (logger){
        ret.log(*logger, max_log10p);
    }
    ret.clear_beyond_log10p(max_log10p);
    return ret;
}

void DictionaryMatcher::add_candidate(Language language, std::string token, const std::u32string& candidate){
    dictionary(language).add_candidate(std::move(token), candidate);
}


}
}
