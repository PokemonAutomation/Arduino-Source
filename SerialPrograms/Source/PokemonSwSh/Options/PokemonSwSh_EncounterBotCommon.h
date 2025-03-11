/*  Encounter Bot Common
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterBotCommon_H
#define PokemonAutomation_PokemonSwSh_EncounterBotCommon_H

#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "EncounterFilter/PokemonSwSh_EncounterFilterOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class EncounterBotCommonOptions : public BatchOption{
public:
    EncounterBotCommonOptions(bool rare_stars, bool enable_overrides)
        : BatchOption(LockMode::UNLOCK_WHILE_RUNNING)
        , FILTER(rare_stars, enable_overrides)
        , VIDEO_ON_SHINY(
            "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
            LockMode::UNLOCK_WHILE_RUNNING,
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

    EncounterFilterOption2 FILTER;
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
