/*  Encounter Mon Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_EncounterBotOptions_H
#define PokemonAutomation_Pokemon_EncounterBotOptions_H

#include "CommonTools/Options/LanguageOCROption.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


class EncounterBotLanguage : public OCR::LanguageOCROption{
public:
    EncounterBotLanguage(bool required = false)
        : LanguageOCROption(
            "<b>Game Language:</b><br>Attempt to read and log the encountered " + STRING_POKEMON + " in this language.<br>Set to \"None\" to disable this feature.",
            PokemonNameReader::instance().languages(),
            LockMode::LOCK_WHILE_RUNNING,
            required
        )
    {}
};



}
}
#endif
