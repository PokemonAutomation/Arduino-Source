/*  Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "DictionaryMatcher.h"

namespace PokemonAutomation{
namespace OCR{


const DictionaryOCR& DictionaryMatcher::dictionary(Language language) const{
    auto iter = m_database.find(language);
    if (iter == m_database.end()){
        PA_THROW_StringException("Language not loaded.");
    }
    return iter->second;
}
DictionaryOCR& DictionaryMatcher::dictionary(Language language){
    SpinLockGuard lg(m_lock, "LargeDictionaryMatcher::dictionary()");
    auto iter = m_database.find(language);
    if (iter == m_database.end()){
        PA_THROW_StringException("Language not loaded.");
    }
    return iter->second;
}


void DictionaryMatcher::match_substring(
    StringMatchResult& results,
    Language language,
    const QString& text,
    double log10p_spread
) const{
    dictionary(language).match_substring(results, text, log10p_spread);
}
void DictionaryMatcher::add_candidate(Language language, std::string token, const QString& candidate){
    dictionary(language).add_candidate(std::move(token), candidate);
}


}
}
