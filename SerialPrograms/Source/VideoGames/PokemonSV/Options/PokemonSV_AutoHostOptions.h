/*  Auto Host Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoHostOptions_H
#define PokemonAutomation_PokemonSV_AutoHostOptions_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class LobbyWaitDelay : public SimpleIntegerOption<uint16_t>{
public:
    LobbyWaitDelay()
        : SimpleIntegerOption<uint16_t>(
            "<b>Lobby Wait Delay (in seconds):</b><br>Wait this long before starting raid. Start time is 3 minutes minus this number.",
            LockMode::UNLOCK_WHILE_RUNNING,
            60, 15, 180
        )
    {}
};

class StartRaidPlayers : public IntegerEnumDropdownOption{
public:
    StartRaidPlayers()
        : IntegerEnumDropdownOption(
            "<b>Start Players:</b><br>Start the raid when there are this many players in the lobby.",
            {
                {2, "2", "2 players including the host(s)"},
                {3, "3", "3 players including the host(s)"},
                {4, "4", "4 players including the host(s)"},
            },
            LockMode::UNLOCK_WHILE_RUNNING,
            4
        )
    {}
};

class ShowRaidCode : public BooleanCheckBoxOption{
public:
    ShowRaidCode()
        : BooleanCheckBoxOption(
            "<b>Show Raid Code:</b><br>Include the raid code in the post notifications. This allows FCE users to copy-paste the code.",
            LockMode::UNLOCK_WHILE_RUNNING,
            true
        )
    {}

};

class AutoHostDescription : public TextEditOption{
public:
    AutoHostDescription()
        : TextEditOption(
            "<b>Description:</b>",
            LockMode::UNLOCK_WHILE_RUNNING,
            "",
            "Auto-Hosting Shiny Eevee"
        )
    {}
};

class RemoteKillSwitch : public StringOption{
public:
    RemoteKillSwitch()
        : StringOption(
            false,
            "<b>Remote Kill Switch:</b><br>Stop the auto-host if the session crosses the date/time specified in this URL. "
            "The default URL is maintained by the PA/SHA staff which updates this with the event change dates.",
            LockMode::UNLOCK_WHILE_RUNNING,
            "https://raw.githubusercontent.com/PokemonAutomation/ServerConfigs-PA-SHA/main/PokemonScarletViolet/TeraAutoHost-KillSwitch.json",
            "https://raw.githubusercontent.com/PokemonAutomation/ServerConfigs-PA-SHA/main/PokemonScarletViolet/TeraAutoHost-KillSwitch.json"
        )
    {}
};

class ConsecutiveFailurePause : public SimpleIntegerOption<uint8_t>{
public:
    ConsecutiveFailurePause()
        : SimpleIntegerOption<uint8_t>(
            "<b>Consecutive Failure Stop/Pause:</b><br>Pause or stop the program if this many consecutive raids fail.<br>"
            "It is not recommended to set this higher than 3 since soft bans start after 3 disconnects.",
            LockMode::UNLOCK_WHILE_RUNNING,
            3, 1
        )
    {}
};

class FailurePauseMinutes : public SimpleIntegerOption<uint8_t>{
public:
    FailurePauseMinutes()
        : SimpleIntegerOption<uint8_t>(
            "<b>Failure Pause Time (in minutes):</b><br>If you trigger the above by failing too many times, "
            "pause for this many minutes before resuming the program. (Zero stops the program.)",
            LockMode::UNLOCK_WHILE_RUNNING,
            0, 0
        )
    {}
};

class RolloverPrevention : public BooleanCheckBoxOption{
public:
    RolloverPrevention()
        : BooleanCheckBoxOption(
            "<b>Rollover Prevention:</b><br>Periodically set the time back to 12AM to prevent the date from rolling over and losing the raid.",
            LockMode::UNLOCK_WHILE_RUNNING,
            true
        )
    {}
};

class RaidPostNotification : public EventNotificationOption{
public:
    RaidPostNotification()
        : EventNotificationOption("Hosting Announcements", true, false, ImageAttachmentMode::JPG, {"LiveHost"})
    {}
};

class RaidStartNotification : public EventNotificationOption{
public:
    RaidStartNotification()
        : EventNotificationOption("Raid Start Announcements", true, false, ImageAttachmentMode::JPG, {"LiveHost"})
    {}
};

class JoinReportNotification : public EventNotificationOption{
public:
    JoinReportNotification()
        : EventNotificationOption("Player Join Reports", true, false, {"Telemetry"})
    {}
};






}
}
}
#endif
