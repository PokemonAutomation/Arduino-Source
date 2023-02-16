/*  Encounter Bot Common
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_EncounterBotCommon_H
#define PokemonAutomation_PokemonSV_EncounterBotCommon_H

#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "PokemonSV_EncounterActionsTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class EncounterBotCommonOptions : public BatchOption{
public:
    EncounterBotCommonOptions()
        : BatchOption(LockWhileRunning::UNLOCKED)
        , VIDEO_ON_SHINY(
            "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
            LockWhileRunning::UNLOCKED,
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
        PA_ADD_OPTION(ACTIONS_TABLE);
        PA_ADD_OPTION(VIDEO_ON_SHINY);
    }

    EncounterActionsTable ACTIONS_TABLE;
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
