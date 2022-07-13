/*  Small Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_SmallDictionaryMatcher_H
#define PokemonAutomation_OCR_SmallDictionaryMatcher_H

#include "OCR_DictionaryMatcher.h"

namespace PokemonAutomation{
    class JsonObject;
namespace OCR{


class SmallDictionaryMatcher : public DictionaryMatcher{
public:
    SmallDictionaryMatcher(const std::string& json_offset, bool first_only = false);
    SmallDictionaryMatcher(const JsonObject& json, bool first_only = false);

    void save(const std::string& json_path) const;
};


}
}
#endif
