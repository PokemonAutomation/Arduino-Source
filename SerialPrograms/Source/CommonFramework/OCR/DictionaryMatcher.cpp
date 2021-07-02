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


MatchResult DictionaryMatcher::match_substring(
    Language language,
    const QString& text
) const{
    return dictionary(language).match_substring(text);
}
MatchResult DictionaryMatcher::match_substring(
    Language language,
    const std::string& expected,
    const QString& text
) const{
    return dictionary(language).match_substring(expected, text);
}
void DictionaryMatcher::add_candidate(Language language, std::string token, const QString& candidate){
    dictionary(language).add_candidate(std::move(token), candidate);
}


}
}
