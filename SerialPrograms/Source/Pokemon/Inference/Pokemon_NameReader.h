/*  Pokemon Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameReader_H
#define PokemonAutomation_Pokemon_PokemonNameReader_H

#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageTypes/ImageReference.h"
#include "CommonFramework/OCR/OCR_LargeDictionaryMatcher.h"

namespace PokemonAutomation{
namespace Pokemon{


class PokemonNameReader : public OCR::LargeDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;

private:
    PokemonNameReader();
public:
    PokemonNameReader(const std::set<std::string>& subset);

public:
    static const PokemonNameReader& instance();

    OCR::StringMatchResult read_substring(
        LoggerQt& logger,
        Language language,
        const ConstImageRef& image
    ) const;

};


}
}
#endif
