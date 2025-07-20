/*  Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_DictionaryMatcher_H
#define PokemonAutomation_CommonTools_OCR_DictionaryMatcher_H

#include <map>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Language.h"
#include "OCR_Routines.h"
#include "OCR_DictionaryOCR.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace OCR{

// Base class for an OCR matcher. It matches a sub-string from a dictionary.
// The construction of the dictionary is left to the derived class.
class DictionaryMatcher{
public:
    const LanguageSet& languages() const{ return m_languages; }


public:
    const DictionaryOCR& dictionary(Language language) const;


    StringMatchResult match_substring(
        Language language,
        const std::string& text, double log10p_spread = 0.50
    ) const;

    // Match a substring from `image`.
    // text_color_ranges: the function will try multiple attempts of separating
    //   text color and background color, and do OCR on the text with background color
    //   removed. The text color range of attempt is specified by each element in
    //   `text_color_ranges`.
    // max_log10p: Threshold for finding any valid match.
    //   The raw OCR result gives a probability for each dictionary key. The probility p
    //   is that if this key is matched by accident. The lower this value, the closer
    //   the match. Apply log10() to it to get log10p.
    //   If all keys' log10p is above this `max_log10p`, there will be no match.
    // log10p_spread: threshold for considering ambiguous match.
    //   It specifies the minimum separation between the best candidate and
    //   2nd best candidate's log10p for it to not be considered ambiguous.
    // min/max_text_ratio: min/max_text_ratio is the range where the ratio of text color
    //   pixel count to background color pixel count must fall into before the matcher
    // even attempts to OCR. This is useful for pruning images with no appearant texts to
    //   reduce expensive OCR computation.
    OCR::StringMatchResult match_substring_from_image_multifiltered(
        Logger* logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double max_log10p, double log10p_spread = 0.5,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;


public:
    //  These functions are thread-safe with themselves, but not with any other
    //  functions in this class.
    DictionaryOCR& dictionary(Language language);
    void add_candidate(Language language, std::string token, const std::u32string& candidate);


protected:
    LanguageSet m_languages;
    std::map<Language, DictionaryOCR> m_database;
    SpinLock m_lock;
};


}
}
#endif
