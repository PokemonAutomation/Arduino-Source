/*  Auto Host Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoHostTools_H
#define PokemonAutomation_PokemonSV_AutoHostTools_H

#include <atomic>
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "PokemonSV/Options/PokemonSV_AutoHostOptions.h"

namespace PokemonAutomation{
    class CancellableScope;
namespace NintendoSwitch{
namespace PokemonSV{



void send_host_announcement(
    ProgramEnvironment& env, VideoStream& host_console,
    const std::string& lobby_code, bool show_raid_code,
    const std::string& description,
    EventNotificationOption& NOTIFICATION_RAID_POST
);


class TeraFailTracker{
public:
    TeraFailTracker(
        ProgramEnvironment& env, CancellableScope& scope,
        EventNotificationOption& notification_error_recoverable,
        ConsecutiveFailurePause& consecutive_failure_pause,
        FailurePauseMinutes& failure_pause_minutes
    );

    void on_raid_start();
    void report_successful_raid();
    void report_raid_error();

private:
    ProgramEnvironment& m_env;
    CancellableScope& m_scope;
    EventNotificationOption& m_notification_error_recoverable;
    ConsecutiveFailurePause& m_consecutive_failure_pause;
    FailurePauseMinutes& m_failure_pause_minutes;
    bool m_completed_one = false;
    size_t m_consecutive_failures = 0;
    std::atomic<bool> m_current_raid_error;
};


class KillSwitchTracker{
public:
    KillSwitchTracker(ProgramEnvironment& env);

    void check_kill_switch(const std::string& kill_switch_url);

private:
    ProgramEnvironment& m_env;
    WallClock m_killswitch_time;
    std::string m_killswitch_reason;
};





}
}
}
#endif
