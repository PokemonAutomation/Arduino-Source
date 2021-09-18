/*  Max Lair Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Options/BatchOption.h"
#include "CommonFramework/OCR/LanguageOptionOCR.h"
#include "Pokemon/Options/Pokemon_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


class MaxLairPlayerOptions : public GroupOption{
public:
    MaxLairPlayerOptions(
        QString label,
        const LanguageSet& languages,
        bool enable_boss_ball
    );

    OCR::LanguageOCR language;
    PokemonBallSelect normal_ball;
    PokemonBallSelect boss_ball;
};


}
}
}
#endif
