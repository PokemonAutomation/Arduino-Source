/*  Pokeball Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokeballNameReader_H
#define PokemonAutomation_Pokemon_PokeballNameReader_H

#include <QImage>
#include "CommonFramework/OCR/SmallDictionaryMatcher.h"

namespace PokemonAutomation{
namespace Pokemon{


class PokeballNameReader : public OCR::SmallDictionaryMatcher{
public:
    PokeballNameReader();

    static PokeballNameReader& instance();

    OCR::MatchResult read_substring(
        Language language,
        const QImage& image
    ) const;
    OCR::MatchResult read_substring(
        Language language,
        const std::string& expected,
        const QImage& image
    ) const;
};


}
}
#endif
