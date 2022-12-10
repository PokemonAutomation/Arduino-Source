/*  Auto Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraBattler.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV_AutoHost.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


AutoHost_Descriptor::AutoHost_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:AutoHost",
        STRING_POKEMON + " SV", "Auto-Host",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/AutoHost.md",
        "Auto-host a Tera raid.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct AutoHost_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_raids(m_stats["Raids"])
        , m_empty(m_stats["Empty Raids"])
        , m_full(m_stats["Full Raids"])
        , m_raiders(m_stats["Total Raiders"])
        , m_wins(m_stats["Wins"])
        , m_losses(m_stats["Losses"])
        , m_banned(m_stats["Banned"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Raids");
        m_display_order.emplace_back("Empty Raids");
        m_display_order.emplace_back("Full Raids");
        m_display_order.emplace_back("Total Raiders");
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Banned", true);
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_empty;
    std::atomic<uint64_t>& m_full;
    std::atomic<uint64_t>& m_raiders;
    std::atomic<uint64_t>& m_wins;
    std::atomic<uint64_t>& m_losses;
    std::atomic<uint64_t>& m_banned;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> AutoHost_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



AutoHost::AutoHost()
    : MODE(
        "<b>Hosting Mode:</b>",
        {
            {Mode::LOCAL,           "local",            "Host Locally"},
            {Mode::ONLINE_CODED,    "online-coded",     "Host Online (link code)"},
            {Mode::ONLINE_EVERYONE, "online-everyone",  "Host Online (everyone)"},
        },
        LockWhileRunning::UNLOCKED,
        Mode::ONLINE_CODED
    )
    , LOBBY_WAIT_DELAY(
        "<b>Lobby Wait Delay (in seconds):</b><br>Wait this long before starting raid. Start time is 3 minutes minus this number.",
        LockWhileRunning::UNLOCKED,
        60, 15, 180
    )
    , START_RAID_PLAYERS(
        "<b>Start Players:</b><br>Start the raid when there are this many players.",
        {
            {2, "2", "2 Players (1 Joiner)"},
            {3, "3", "3 Players (2 Joiners)"},
            {4, "4", "4 Players (3 Joiners)"},
        },
        LockWhileRunning::UNLOCKED,
        4
    )
    , ROLLOVER_PREVENTION(
        "<b>Rollover Prevention:</b><br>Periodically set the time back to 12AM to prevent the date from rolling over and losing the raid.",
        LockWhileRunning::UNLOCKED,
        true
    )
    , DESCRIPTION(
        "<b>Description:</b>",
        LockWhileRunning::UNLOCKED,
        "",
        "Auto-Hosting Shiny Eevee"
    )
    , CONSECUTIVE_FAILURE_PAUSE(
        "<b>Consecutive Failure Stop/Pause:</b><br>Pause or stop the program if this many consecutive raids fail.<br>"
        "It is not recommended to set this higher than 3 since soft bans start after 3 disconnects.",
        LockWhileRunning::UNLOCKED,
        3, 1
    )
    , FAILURE_PAUSE_MINUTES(
        "<b>Failure Pause Time (in minutes):</b><br>If you trigger the above by failing too many times, "
        "pause for this many minutes before resuming the program. (Zero stops the program.)",
        LockWhileRunning::UNLOCKED,
        0, 0
    )
    , TRY_TO_TERASTILIZE(
        "<b>Try to Terastillize:</b><br>Try to terastilize if available. Add 4s per try but greatly increase win rate.",
        LockWhileRunning::UNLOCKED, true
    )
    , BAN_LIST(LockWhileRunning::UNLOCKED)
    , IGNORE_WHITELIST(
        "<b>Ignore Whitelist:</b><br>Ignore the developer whitelist.",
        LockWhileRunning::UNLOCKED,
        false
    )
    , NOTIFICATION_RAID_POST("Hosting Announcements", true, false, ImageAttachmentMode::JPG, {"LiveHost"})
    , NOTIFICATION_RAID_START("Raid Start Announcements", true, false, ImageAttachmentMode::JPG, {"LiveHost"})
    , NOTIFICATIONS({
        &NOTIFICATION_RAID_POST,
        &NOTIFICATION_RAID_START,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(LOBBY_WAIT_DELAY);
    PA_ADD_OPTION(START_RAID_PLAYERS);
    PA_ADD_OPTION(ROLLOVER_PREVENTION);
    PA_ADD_OPTION(DESCRIPTION);
    PA_ADD_OPTION(CONSECUTIVE_FAILURE_PAUSE);
    PA_ADD_OPTION(FAILURE_PAUSE_MINUTES);
    PA_ADD_OPTION(TRY_TO_TERASTILIZE);
    PA_ADD_OPTION(BAN_LIST);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(IGNORE_WHITELIST);
    }
    PA_ADD_OPTION(NOTIFICATIONS);
}



WallClock AutoHost::wait_for_lobby_open(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    VideoOverlaySet overlays(env.console.overlay());

    TeraLobbyWatcher lobby(COLOR_RED);
    lobby.make_overlays(overlays);

    int ret = wait_until(
        env.console, context,
        std::chrono::seconds(60),
        {lobby}
    );
    if (ret < 0){
        dump_image_and_throw_recoverable_exception(
            env, env.console,
            NOTIFICATION_ERROR_RECOVERABLE,
            "NoLobby", "Unable to detect Tera lobby after 60 seconds."
        );
    }
    WallClock start_time = current_time();
    context.wait_for(std::chrono::seconds(1));

    //  Send notification.
    std::vector<std::pair<std::string, std::string>> messages;
    std::string description = DESCRIPTION;
    if (!description.empty()){
        messages.emplace_back("Description:", std::move(description));
    }

    VideoSnapshot snapshot = env.console.video().snapshot();
    std::string code = lobby.raid_code(env.logger(), env.program_info(), snapshot);
    if (!code.empty()){
        messages.emplace_back("Raid Code:", std::move(code));
    }

    send_program_notification(
        env, NOTIFICATION_RAID_POST,
        Color(0),
        "Tera Raid Notification",
        messages, "",
        env.console.video().snapshot()
    );

    return start_time;
}
void AutoHost::update_stats_on_raid_start(SingleSwitchProgramEnvironment& env, uint8_t player_count){
    AutoHost_Descriptor::Stats& stats = env.current_stats<AutoHost_Descriptor::Stats>();

    player_count = std::max<uint8_t>(player_count, 1);

    if (player_count == 4){
        stats.m_full++;
    }
    if (player_count == 1){
        stats.m_empty++;
    }
    stats.m_raiders += player_count - 1;
}
bool AutoHost::start_raid(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    WallClock start_time,
    uint8_t player_count
){
    AutoHost_Descriptor::Stats& stats = env.current_stats<AutoHost_Descriptor::Stats>();

    //  This is the state machine to actually start the raid.

    while (true){
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        WhiteScreenOverWatcher start_raid(COLOR_BLUE);
        BattleMenuWatcher battle_menu(COLOR_CYAN);
        context.wait_for_all_requests();
        int ret = run_until(
            env.console, context,
            [start_time](BotBaseContext& context){
                while (true){
                    pbf_press_button(context, BUTTON_A, 20, 105);
                    context.wait_for_all_requests();
                    if (current_time() > start_time + std::chrono::minutes(4)){
                        return;
                    }
                }
            },
            {dialog, start_raid, battle_menu}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            env.log("Raid timed out!", COLOR_ORANGE);
            stats.m_empty++;
            return false;
        case 1:
            env.log("Raid started! (white screen)", COLOR_BLUE);
            update_stats_on_raid_start(env, player_count);
            return true;
        case 2:
            env.log("Raid started! (battle menu)", COLOR_BLUE);
            update_stats_on_raid_start(env, player_count);
            return true;
        default:
            dump_image_and_throw_recoverable_exception(
                env, env.console,
                NOTIFICATION_ERROR_RECOVERABLE,
                "StuckLobby", "Stuck in lobby for 4 minutes."
            );
        }
    }
}
bool AutoHost::run_lobby(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoHost_Descriptor::Stats& stats = env.current_stats<AutoHost_Descriptor::Stats>();

    VideoOverlaySet overlays(env.console.overlay());

    WallClock start_time = wait_for_lobby_open(env, context);

    //  This state machine is while the lobby is running and we haven't decided
    //  whether to start or cancel the raid.

    uint8_t last_known_player_count = 1;
    while (true){
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        WhiteScreenOverWatcher start_raid(COLOR_BLUE);
        TeraLobbyReadyWaiter ready(COLOR_RED, (uint8_t)START_RAID_PLAYERS.current_value());
        TeraLobbyBanWatcher ban_watcher(env.logger(), COLOR_RED, BAN_LIST, true, IGNORE_WHITELIST);
        context.wait_for_all_requests();
        WallClock end_time = start_time + std::chrono::seconds(LOBBY_WAIT_DELAY);
        int ret = wait_until(
            env.console, context,
            end_time,
            {
                dialog, start_raid, ready,
                {ban_watcher, std::chrono::seconds(1)}
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        int8_t current_players = ready.last_known_player_count();
        if (current_players > 0){
            last_known_player_count = current_players;
        }
        env.log("Currently there are " + std::to_string(last_known_player_count) + " player(s).");
        VideoSnapshot snapshot = env.console.video().snapshot();
        std::vector<TeraLobbyNameMatchResult> detected_banned_players;
        switch (ret){
        case 0:
            env.log("Raid timed out!", COLOR_ORANGE);
//            pbf_press_button(context, BUTTON_B, 20, 230);
            stats.m_empty++;
            return false;
        case 1:
            env.log("Raid started!", COLOR_BLUE);
            update_stats_on_raid_start(env, last_known_player_count);
            return true;
        case 2:
            env.log("Enough players joined, attempting to start raid!", COLOR_BLUE);
            ban_watcher.process_frame(snapshot, snapshot.timestamp);
            detected_banned_players = ban_watcher.detected_banned_players();
            if (detected_banned_players.empty()){
                send_program_notification(
                    env, NOTIFICATION_RAID_START,
                    COLOR_GREEN,
                    "Tera Raid is Starting!",
                    {{
                        "Start Reason:",
                        "Lobby has reached " + std::to_string(current_players) + " players."
                    }}, "",
                    snapshot
                );
                break;
            }
            //  Intentional fall-through.
        case 3:{
            env.log("Detected banned user!", COLOR_RED);
            if (detected_banned_players.empty()){
                detected_banned_players = ban_watcher.detected_banned_players();
            }
            std::string message;
            for (const TeraLobbyNameMatchResult& user : detected_banned_players){
                env.log("Banned User: " + user.to_str(), COLOR_RED);
                message += user.to_str();
                message += "\n";
                if (!user.notes.empty()){
                    message += "Reason: " + user.notes;
                    message += "\n";
                }
            }
            stats.m_banned += detected_banned_players.size();
            send_program_notification(
                env, NOTIFICATION_RAID_POST,
                COLOR_RED,
                "Raid Canceled Due to Banned User",
                {{"Banned User(s):", std::move(message)}}, "",
                snapshot
            );
            pbf_press_button(context, BUTTON_B, 20, 230);
            pbf_press_button(context, BUTTON_A, 20, 230);
            return false;
        }
        default:
            if (current_time() - start_time > std::chrono::minutes(4)){
                dump_image_and_throw_recoverable_exception(
                    env, env.console,
                    NOTIFICATION_ERROR_RECOVERABLE,
                    "StuckLobby", "Stuck in lobby for 4 minutes."
                );
            }
            env.log("Timeout reached. Starting raid now...", COLOR_PURPLE);
            send_program_notification(
                env, NOTIFICATION_RAID_START,
                COLOR_GREEN,
                "Tera Raid is Starting!",
                {{
                    "Start Reason:",
                    "Waited more than " + std::to_string(LOBBY_WAIT_DELAY) + " seconds."
                }}, "",
                snapshot
            );
        }
        break;
    }

    return start_raid(env, context, start_time, last_known_player_count);
}

void AutoHost::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    AutoHost_Descriptor::Stats& stats = env.current_stats<AutoHost_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 10);

    bool skip_reset = true;
    bool completed_one = false;
    size_t consecutive_failures = 0;
    WallClock last_time_fix = WallClock::min();
    while (true){
        env.update_stats();

        if (consecutive_failures > 0 && !completed_one){
            throw OperationFailedException(env.logger(), "Failed 1st raid attempt. Will not retry due to risk of ban.");
        }
        size_t fail_threshold = CONSECUTIVE_FAILURE_PAUSE;
        if (consecutive_failures >= fail_threshold){
            uint16_t minutes = FAILURE_PAUSE_MINUTES;
            if (minutes == 0){
                throw OperationFailedException(
                    env.logger(),
                    "Failed " + std::to_string(fail_threshold) +  " raid(s) in the row. "
                    "Stopping to prevent possible ban."
                );
            }else{
                send_program_recoverable_error_notification(
                    env, NOTIFICATION_ERROR_RECOVERABLE,
                    "Failed " + std::to_string(fail_threshold) +  " raid(s) in the row. "
                    "Pausing program for " + std::to_string(minutes) + " minute(s)."
                );
                context.wait_for(std::chrono::minutes(minutes));
                consecutive_failures = 0;
            }
        }

        if (!skip_reset){
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            if (ROLLOVER_PREVENTION){
                WallClock now = current_time();
                if (last_time_fix == WallClock::min() || now - last_time_fix > std::chrono::hours(4)){
                    set_time_to_12am_from_home(env.console, context);
                    last_time_fix = now;
                }
            }
            reset_game_from_home(env, env.console, context, 5 * TICKS_PER_SECOND);
        }
        skip_reset = false;

        //  Store the mode locally in case the user changes in the middle of
        //  this iteration.
        Mode mode = MODE;

        if (mode != Mode::LOCAL){
            //  Connect to internet.
//            throw InternalProgramError(&env.logger(), PA_CURRENT_FUNCTION, "Online mode has not been implemented yet.");
            try{
                connect_to_internet_from_overworld(env.console, context);
            }catch (OperationFailedException&){
                consecutive_failures++;
                stats.m_errors++;
                continue;
            }
        }

        if (!open_raid(env.console, context)){
            env.log("No Tera raid found.", COLOR_RED);
            consecutive_failures++;
            continue;
        }
        env.log("Tera raid found!", COLOR_BLUE);

        context.wait_for(std::chrono::milliseconds(100));

        pbf_press_button(context, BUTTON_A, 20, 230);
        if (mode != Mode::LOCAL){
            if (mode == Mode::ONLINE_EVERYONE){
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
            }
            pbf_press_button(context, BUTTON_A, 20, 230);
        }

        context.wait_for_all_requests();
        try{
            if (!run_lobby(env, context)){
                continue;
            }
            env.update_stats();

            stats.m_raids++;
            bool win = run_tera_battle(env, env.console, context, NOTIFICATION_ERROR_RECOVERABLE, TRY_TO_TERASTILIZE);
            env.update_stats();
            if (win){
                stats.m_wins++;
                exit_tera_win_without_catching(env.console, context);
            }else{
                stats.m_losses++;
            }
            completed_one = true;
            consecutive_failures = 0;
        }catch (OperationFailedException&){
            consecutive_failures++;
            stats.m_errors++;
            continue;
        }
    }
}



}
}
}
