/*  Large Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_LargeDictionaryMatcher_H
#define PokemonAutomation_OCR_LargeDictionaryMatcher_H

#include "DictionaryMatcher.h"

namespace PokemonAutomation{
namespace OCR{


class LargeDictionaryMatcher : public DictionaryMatcher{
public:
    LargeDictionaryMatcher(
        const QString& json_file_prefix,
        const std::set<std::string>* subset,    //  Only include tokens in this set.
        bool first_only
    );

    void save(Language language, const QString& json_path) const;
//    void update(Language language) const;


private:
//    QString m_prefix;
};


}
}
#endif
