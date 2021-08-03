/*  Encounter Mon Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_EncounterBotOptions_H
#define PokemonAutomation_Pokemon_EncounterBotOptions_H

#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/EnumDropdown.h"
#include "CommonFramework/Options/LanguageOCR.h"
#include "Pokemon/Pokemon_NameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


class EncounterBotLanguage : public LanguageOCR{
public:
    EncounterBotLanguage(const PokemonNameReader& name_reader, bool required = false)
        : LanguageOCR(
            "<b>Game Language:</b><br>Attempt to read and log the encountered " + STRING_POKEMON + " in this language.<br>Set to \"None\" to disable this feature.",
            name_reader.languages(), required
        )
    {}
};


enum class EncounterBotNotificationLevel{
    NO_NOTIFICATIONS,
    SHINY_ONLY,
    PERIODIC_AND_SHINY,
    EVERYTHING,
};

class EncounterBotNotifications : public EnumDropdown{
public:
    EncounterBotNotifications()
        : EnumDropdown(
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



}
}
#endif
