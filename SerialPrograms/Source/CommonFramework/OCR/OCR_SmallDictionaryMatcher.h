/*  Small Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_SmallDictionaryMatcher_H
#define PokemonAutomation_OCR_SmallDictionaryMatcher_H

#include <QJsonObject>
#include "OCR_DictionaryMatcher.h"

namespace PokemonAutomation{
namespace OCR{


class SmallDictionaryMatcher : public DictionaryMatcher{
public:
    SmallDictionaryMatcher(const QString& json_offset, bool first_only = false);
    SmallDictionaryMatcher(const QJsonObject& json, bool first_only = false);

    void save(const QString& json_path) const;
};


}
}
#endif
