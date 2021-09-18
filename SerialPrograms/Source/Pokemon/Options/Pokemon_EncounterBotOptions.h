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
#include "CommonFramework/OCR/LanguageOptionOCR.h"
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


enum class EncounterBotNotificationLevel{
    NO_NOTIFICATIONS,
    SHINY_ONLY,
    PERIODIC_AND_SHINY,
    EVERYTHING,
};
class EncounterBotNotifications : public EnumDropdownOption{
public:
    EncounterBotNotifications()
        : EnumDropdownOption(
            "<b>Discord Notification Level:</b><br>Requires Discord notification settings be set.",
            {
                "No notifications.",
                "Notify on shinies only.",
                "Notify once an hour and on shinies.",
                "Notify on all encounters."
            },
            2
        )
    {}

    operator EncounterBotNotificationLevel() const{
        return (EncounterBotNotificationLevel)(size_t)*this;
    }
};


class EncounterBotScreenshotOption : public ScreenshotOption{
public:
    EncounterBotScreenshotOption()
        : ScreenshotOption(
            "<b>Attach Shiny Screenshot:</b><br>Attach screenshot of shiny encounters to notification."
        )
    {}
};



}
}
#endif
