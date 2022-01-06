/*  Encounter Bot Common
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterBotCommon_H
#define PokemonAutomation_PokemonBDSP_EncounterBotCommon_H

#include "CommonFramework/Options/BatchOption/BatchOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "EncounterFilter/PokemonBDSP_EncounterFilterOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



class EncounterBotCommonOptions : public BatchOption{
public:
    EncounterBotCommonOptions(bool enable_overrides, bool allow_autocatch)
        : FILTER(enable_overrides, allow_autocatch)
        , VIDEO_ON_SHINY(
            "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
            true
        )
        , NOTIFICATION_NONSHINY(
            "Non-Shiny Encounter",
            true, false,
            {"Notifs"},
            std::chrono::seconds(3600)
        )
        , NOTIFICATION_SHINY(
            "Shiny Encounter",
            true, true, ImageAttachmentMode::JPG,
            {"Notifs", "Showcase"}
        )
        , NOTIFICATION_CATCH_SUCCESS(
            "Catch Success",
            true, false,
            {"Notifs"}
        )
        , NOTIFICATION_CATCH_FAILED(
            "Catch Failed",
            true, true,
            {"Notifs"}
        )
    {
        PA_ADD_OPTION(FILTER);
        PA_ADD_OPTION(VIDEO_ON_SHINY);
    }

    EncounterFilterOption FILTER;
    BooleanCheckBoxOption VIDEO_ON_SHINY;

    EventNotificationOption NOTIFICATION_NONSHINY;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_CATCH_SUCCESS;
    EventNotificationOption NOTIFICATION_CATCH_FAILED;
};




}
}
}
#endif
