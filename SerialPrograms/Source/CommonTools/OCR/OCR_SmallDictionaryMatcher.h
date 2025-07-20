/*  Small Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_SmallDictionaryMatcher_H
#define PokemonAutomation_CommonTools_OCR_SmallDictionaryMatcher_H

#include "OCR_DictionaryMatcher.h"

namespace PokemonAutomation{
    class JsonObject;
namespace OCR{

// An OCR matcher with a small dictionary.
// Since the dictionary is small, all languages of this dictionary are stored in
// the same JSON file.
class SmallDictionaryMatcher : public DictionaryMatcher{
public:
    // Load the JSON file to build the dictionary for all languages stored in the JSON.
    // first_only: whether to use only the first value for each slug.
    //   This parameter is only useful for training the OCR.
    //   During inference, we set `first_only` to false so that each slug corresponds to
    //   all the values in that dictionary entry.
    //   e.g. if `first_only` is false, when loading Pokemon Abomasnow for traditional Chinese,
    //   "abomasnow": ["暴雪王", "繁霎王", "鬘粘王"],
    //   when OCR detects any of "暴雪王", "繁霎王" or "鬘粘王", the matcher returns Abomasnow.
    //   if 'first_only' is true, only when OCR detects "暴雪王" do the matcher return Abomasnow.
    SmallDictionaryMatcher(const std::string& json_path, bool first_only = false);
    // Load JSON object to build the dictionary for all languages stored in the JSON.
    // See the comments of 
    // `SmallDictionaryMatcher(const std::string& json_path, bool first_only = false)`
    // to konw the usage of `first_only`.
    SmallDictionaryMatcher(const JsonObject& json, bool first_only = false);

    void save(const std::string& json_path) const;
};


}
}
#endif
