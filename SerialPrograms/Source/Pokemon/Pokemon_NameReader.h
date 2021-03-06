/*  Pokemon Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameReader_H
#define PokemonAutomation_Pokemon_PokemonNameReader_H

#include <QImage>
#include "CommonFramework/OCR/LargeDictionaryMatcher.h"

namespace PokemonAutomation{
namespace Pokemon{


class PokemonNameReader : public OCR::LargeDictionaryMatcher{
public:
    PokemonNameReader();

    OCR::MatchResult read_exact(
        Language language,
        const QImage& image
    ) const;
    OCR::MatchResult read_exact(
        Language language,
        const std::string& expected,
        const QImage& image
    ) const;
};


}
}
#endif
