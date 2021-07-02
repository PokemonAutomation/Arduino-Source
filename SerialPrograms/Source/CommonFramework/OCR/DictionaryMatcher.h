/*  Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_DictionaryMatcher_H
#define PokemonAutomation_OCR_DictionaryMatcher_H

#include <map>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Language.h"
#include "DictionaryOCR.h"

namespace PokemonAutomation{
namespace OCR{


class DictionaryMatcher{
public:
    const LanguageSet& languages() const{ return m_languages; }


public:
    const DictionaryOCR& dictionary(Language language) const;

    MatchResult match_substring(
        Language language,
        const QString& text
    ) const;
    MatchResult match_substring(
        Language language,
        const std::string& expected,
        const QString& text
    ) const;


public:
    //  These functions are thread-safe with themselves, but not with any other
    //  functions in this class.
    DictionaryOCR& dictionary(Language language);
    void add_candidate(Language language, std::string token, const QString& candidate);


protected:
    LanguageSet m_languages;
    std::map<Language, DictionaryOCR> m_database;
    SpinLock m_lock;
};


}
}
#endif
