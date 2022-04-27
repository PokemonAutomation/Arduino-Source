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
#include "OCR_Routines.h"
#include "OCR_DictionaryOCR.h"

namespace PokemonAutomation{
namespace OCR{


class DictionaryMatcher{
public:
    const LanguageSet& languages() const{ return m_languages; }


public:
    const DictionaryOCR& dictionary(Language language) const;

    StringMatchResult match_substring(
        Language language,
        const QString& text, double log10p_spread = 0.50
    ) const;

    OCR::StringMatchResult match_substring_from_image_multifiltered(
        LoggerQt& logger,
        Language language,
        const ConstImageRef& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double max_log10p, double log10p_spread = 0.5,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
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
