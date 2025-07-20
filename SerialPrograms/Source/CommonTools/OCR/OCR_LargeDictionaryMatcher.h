/*  Large Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_LargeDictionaryMatcher_H
#define PokemonAutomation_CommonTools_OCR_LargeDictionaryMatcher_H

#include "OCR_DictionaryMatcher.h"

namespace PokemonAutomation{
namespace OCR{

// An OCR matcher with a small dictionary.
// Since the dictionary is large, each languages of the dictionary is stored as a
// separate JSON file.
class LargeDictionaryMatcher : public DictionaryMatcher{
public:
    // Load the JSON file to build the dictionary for all languages stored in each
    // language JSON file.
    // json_file_prefix: the file path prefix for all the language JSON files.
    // subset: if not nullptr, only add keys in this `subset` to the dictionary.
    //   otherwise, add all keys found in the JSON to the dictionary.
    // first_only: whether to use only the first value for each slug.
    //   This parameter is only useful for training the OCR.
    //   During inference, we set `first_only` to false so that each slug corresponds to
    //   all the values in that dictionary entry.
    //   e.g. if `first_only` is false, when loading Pokemon Abomasnow for traditional Chinese,
    //   "abomasnow": ["暴雪王", "繁霎王", "鬘粘王"],
    //   when OCR detects any of "暴雪王", "繁霎王" or "鬘粘王", the matcher returns Abomasnow.
    //   if 'first_only' is true, only when OCR detects "暴雪王" do the matcher return Abomasnow.
    LargeDictionaryMatcher(
        const std::string& json_file_prefix,
        const std::set<std::string>* subset,
        bool first_only
    );

    void save(Language language, const std::string& json_path) const;

private:
};


}
}
#endif
