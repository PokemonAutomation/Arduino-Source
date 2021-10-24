/*  Pokemon Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameReader_H
#define PokemonAutomation_Pokemon_PokemonNameReader_H

#include <QImage>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/OCR/LargeDictionaryMatcher.h"

namespace PokemonAutomation{
namespace Pokemon{


class PokemonNameReader : public OCR::LargeDictionaryMatcher{
    static constexpr double MAX_LOG10P = -1.40;

private:
    PokemonNameReader();
public:
    PokemonNameReader(const std::set<std::string>& subset);

public:
    static const PokemonNameReader& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const QImage& image
    ) const;

};


}
}
#endif
