/*  Encounter Mon Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_EncounterBotOptions_H
#define PokemonAutomation_Pokemon_EncounterBotOptions_H

#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


class EncounterBotLanguage : public OCR::LanguageOCR{
public:
    EncounterBotLanguage(bool required = false)
        : LanguageOCR(
            "<b>Game Language:</b><br>Attempt to read and log the encountered " + STRING_POKEMON + " in this language.<br>Set to \"None\" to disable this feature.",
            PokemonNameReader::instance().languages(), required
        )
    {}
};



}
}
#endif
