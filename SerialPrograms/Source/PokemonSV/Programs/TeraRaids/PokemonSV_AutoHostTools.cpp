/*  Auto Host Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/TimeQt.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/FileDownloader.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "PokemonSV/Options/PokemonSV_AutoHostOptions.h"
#include "PokemonSV_AutoHostTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



void send_host_announcement(
    ProgramEnvironment& env, VideoStream& host_console,
    const std::string& lobby_code, bool show_raid_code,
    const std::string& description,
    EventNotificationOption& NOTIFICATION_RAID_POST
){
    std::vector<std::pair<std::string, std::string>> messages;
    if (!description.empty()){
        messages.emplace_back("Description:", std::move(description));
    }

    VideoSnapshot snapshot = host_console.video().snapshot();
    if (show_raid_code && !lobby_code.empty()){
        messages.emplace_back("Raid Code:", lobby_code);
    }

    send_program_notification(
        env, NOTIFICATION_RAID_POST,
        Color(0),
        "Tera Raid Notification",
        messages, "",
        snapshot
    );
}



TeraFailTracker::TeraFailTracker(
    ProgramEnvironment& env, CancellableScope& scope,
    EventNotificationOption& notification_error_recoverable,
    ConsecutiveFailurePause& consecutive_failure_pause,
    FailurePauseMinutes& failure_pause_minutes
)
    : m_env(env)
    , m_scope(scope)
    , m_notification_error_recoverable(notification_error_recoverable)
    , m_consecutive_failure_pause(consecutive_failure_pause)
    , m_failure_pause_minutes(failure_pause_minutes)
    , m_current_raid_error(false)
{}

void TeraFailTracker::on_raid_start(){
    if (!m_current_raid_error.load(std::memory_order_relaxed)){
        return;
    }
    m_current_raid_error.store(false, std::memory_order_relaxed);

#if 1
    if (m_consecutive_failures > 0 && !m_completed_one){
        throw_and_log<FatalProgramException>(
            m_env.logger(),
            ErrorReport::NO_ERROR_REPORT,
            "Failed 1st raid attempt. Will not retry due to risk of ban."
        );
    }
#endif

    size_t fail_threshold = m_consecutive_failure_pause;
    if (m_consecutive_failures >= fail_threshold){
        uint16_t minutes = m_failure_pause_minutes;
        if (minutes == 0){
            throw_and_log<FatalProgramException>(
                m_env.logger(),
                ErrorReport::NO_ERROR_REPORT,
                "Failed " + std::to_string(fail_threshold) +  " raid(s) in the row. "
                "Stopping to prevent possible ban."
            );
        }else{
            send_program_recoverable_error_notification(
                m_env, m_notification_error_recoverable,
                "Failed " + std::to_string(fail_threshold) +  " raid(s) in the row. "
                "Pausing program for " + std::to_string(minutes) + " minute(s)."
            );
            WallClock start_time = current_time();
            while (current_time() < start_time + std::chrono::minutes(m_failure_pause_minutes)){
                m_scope.wait_for(std::chrono::seconds(1));
            }
            m_consecutive_failures = 0;
        }
    }
}
void TeraFailTracker::report_successful_raid(){
    m_completed_one = true;
    m_consecutive_failures = 0;
}
void TeraFailTracker::report_raid_error(){
    bool expected = false;
    if (m_current_raid_error.compare_exchange_strong(expected, true)){
        m_consecutive_failures++;
        m_env.log("Consecutive Failure Count: " + std::to_string(m_consecutive_failures), COLOR_RED);
    }
}




KillSwitchTracker::KillSwitchTracker(ProgramEnvironment& env)
    : m_env(env)
{}
void KillSwitchTracker::check_kill_switch(const std::string& kill_switch_url){
    if (kill_switch_url.empty()){
        return;
    }

    WallClock start_time = m_env.program_info().start_time;

    if (kill_switch_url.ends_with(".txt")){
        m_env.log("Loading remote kill switch time...");
        try{
            std::string kill_time_str = FileDownloader::download_file(m_env.logger(), kill_switch_url);
            m_killswitch_time = parse_utc_time_str(kill_time_str);
        }catch (OperationFailedException& e){
            m_env.log("Unable to load kill switch URL: " + e.message(), COLOR_RED);
        }catch (ParseException& e){
            m_env.log("Unable to load kill switch URL: " + e.message(), COLOR_RED);
        }
    }else if (kill_switch_url.ends_with(".json")){
        m_env.log("Loading remote kill switch time...");

        try{
            JsonValue json = FileDownloader::download_json_file(m_env.logger(), kill_switch_url);
            const JsonObject* obj = json.to_object();
            if (obj == nullptr){
                throw ParseException("Invalid kill-switch Json.");
            }
            const std::string* date = obj->get_string("date");
            if (date == nullptr){
                m_env.log("Invalid Kill Switch Json", COLOR_RED);
            }else{
                m_killswitch_time = parse_utc_time_str(*date);
            }
            const std::string* reason = obj->get_string("reason");
            if (date == nullptr){
                m_env.log("Invalid Kill Switch Json", COLOR_RED);
            }else{
                m_killswitch_reason = *reason;
            }
        }catch (OperationFailedException& e){
            m_env.log("Invalid kill-switch JSON: " + e.message(), COLOR_RED);
        }catch (ParseException& e){
            m_env.log("Invalid kill-switch JSON: " + e.message(), COLOR_RED);
        }
    }else{
        throw UserSetupError(m_env.logger(), "Invalid kill switch URL extension.");
    }

    WallClock now = current_time();
    m_env.log(
        "Start UTC: " + to_utc_time_str(start_time) +
        ", Current UTC: " + to_utc_time_str(now) +
        ", Kill UTC: " + to_utc_time_str(m_killswitch_time)
    );
    if (start_time < m_killswitch_time && now > m_killswitch_time){
        if (m_killswitch_reason.empty()){
            throw_and_log<FatalProgramException>(
                m_env.logger(),
                ErrorReport::NO_ERROR_REPORT,
                "Stopped by remote kill switch. No reason specified."
            );
        }else{
            throw_and_log<FatalProgramException>(
                m_env.logger(),
                ErrorReport::NO_ERROR_REPORT,
                "Stopped by remote kill switch. Reason: " + m_killswitch_reason
            );
        }
    }
}













}
}
}
