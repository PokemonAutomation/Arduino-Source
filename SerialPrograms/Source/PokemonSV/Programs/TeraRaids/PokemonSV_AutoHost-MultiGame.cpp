/*  Auto Host Multi Game
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
//#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
//#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
//#include "CommonFramework/OCR/OCR_StringNormalization.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_ConnectToInternet.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/FastCodeEntry/PokemonSV_CodeEntry.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraBattler.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV_AutoHostTools.h"
#include "PokemonSV_AutoHostLobbyWaiter.h"
#include "PokemonSV_AutoHost-MultiGame.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


AutoHostMultiGame_Descriptor::AutoHostMultiGame_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:AutoHostMultiGame",
        STRING_POKEMON + " SV", "AutoHost-MultiGame",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/AutoHostMultiGame.md",
        "Auto-host a Tera raid.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct AutoHostMultiGame_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> AutoHostMultiGame_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



AutoHostMultiGame::AutoHostMultiGame()
    : SingleSwitchProgramInstance({"Notifs", "LiveHost"})
    , MODE(
        "<b>Hosting Mode:</b>",
        {
            {HostingMode::LOCAL,            "local",            "Host Locally"},
            {HostingMode::ONLINE_CODED,     "online-coded",     "Host Online (link code)"},
            {HostingMode::ONLINE_EVERYONE,  "online-everyone",  "Host Online (everyone)"},
        },
        LockWhileRunning::UNLOCKED,
        HostingMode::ONLINE_CODED
    )
    , GAME_LIST()
    , NOTIFICATIONS0({
        &NOTIFICATION_RAID_POST,
        &NOTIFICATION_RAID_START,
        &NOTIFICATION_JOIN_REPORT,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(MODE);

    //  General Auto-Hosting Options
    PA_ADD_OPTION(LOBBY_WAIT_DELAY);
    PA_ADD_OPTION(START_RAID_PLAYERS);
    PA_ADD_OPTION(SHOW_RAID_CODE);
    PA_ADD_OPTION(GAME_LIST);
    PA_ADD_OPTION(DESCRIPTION);
    PA_ADD_OPTION(REMOTE_KILL_SWITCH0);
    PA_ADD_OPTION(CONSECUTIVE_FAILURE_PAUSE);
    PA_ADD_OPTION(FAILURE_PAUSE_MINUTES);

    PA_ADD_OPTION(ROLLOVER_PREVENTION);
    PA_ADD_OPTION(BATTLE_AI);

    //  Extended Auto-Hosting Options
    PA_ADD_OPTION(BAN_LIST);
    PA_ADD_OPTION(JOIN_REPORT);

    PA_ADD_OPTION(NOTIFICATIONS0);
}



WallClock AutoHostMultiGame::wait_for_lobby_open(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    std::string& lobby_code
){
    VideoOverlaySet overlays(env.console.overlay());

    TeraLobbyWatcher lobby(env.logger(), env.realtime_dispatcher(), COLOR_RED);
    lobby.make_overlays(overlays);

    int ret = wait_until(
        env.console, context,
        std::chrono::seconds(60),
        {{lobby, std::chrono::milliseconds(500)}}
    );
    if (ret < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Unable to detect Tera lobby after 60 seconds.",
            true
        );
    }
    WallClock start_time = current_time();
    context.wait_for(std::chrono::seconds(1));

    VideoSnapshot snapshot = env.console.video().snapshot();
    lobby_code = lobby.raid_code(env.logger(), env.inference_dispatcher(), snapshot);
    std::string code = lobby.raid_code(env.logger(), env.inference_dispatcher(), snapshot);
    normalize_code(lobby_code, code);

    send_host_announcement(
        env, env.console,
        lobby_code,
        SHOW_RAID_CODE,
        DESCRIPTION,
        NOTIFICATION_RAID_POST
    );

    return start_time;
}
void AutoHostMultiGame::update_stats_on_raid_start(SingleSwitchProgramEnvironment& env, uint8_t player_count){
    AutoHostMultiGame_Descriptor::Stats& stats = env.current_stats<AutoHostMultiGame_Descriptor::Stats>();

    player_count = std::max<uint8_t>(player_count, 1);

    if (player_count == 4){
        stats.m_full++;
    }
    if (player_count == 1){
        stats.m_empty++;
    }
    stats.m_raiders += player_count - 1;
}
bool AutoHostMultiGame::start_raid(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    WallClock start_time,
    uint8_t player_count
){
    AutoHostMultiGame_Descriptor::Stats& stats = env.current_stats<AutoHostMultiGame_Descriptor::Stats>();

    //  This is the state machine to actually start the raid.

    while (true){
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        WhiteScreenOverWatcher start_raid(COLOR_BLUE);
        TeraBattleMenuWatcher battle_menu(COLOR_CYAN);
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
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Stuck in lobby for 4 minutes.",
                true
            );
        }
    }
}


bool AutoHostMultiGame::run_lobby(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    std::string& lobby_code,
    std::array<std::map<Language, std::string>, 4>& player_names
){
    AutoHostMultiGame_Descriptor::Stats& stats = env.current_stats<AutoHostMultiGame_Descriptor::Stats>();

    WallClock start_time = wait_for_lobby_open(env, context, lobby_code);

    TeraLobbyWaiter waiter(
        env, env.console, context,
        1,
        lobby_code, start_time,
        LOBBY_WAIT_DELAY,
        START_RAID_PLAYERS,
        NOTIFICATION_RAID_START,
        BAN_LIST,
        JOIN_REPORT
    );

    TeraLobbyWaiter::LobbyResult result = waiter.run_lobby();
    player_names = waiter.names();

    if (result == TeraLobbyWaiter::LobbyResult::BANNED_PLAYER){
        stats.m_banned++;
    }
    if (result != TeraLobbyWaiter::LobbyResult::RAID_STARTED){
        return false;
    }

    return start_raid(env, context, start_time, waiter.last_known_players());
}

void AutoHostMultiGame::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    AutoHostMultiGame_Descriptor::Stats& stats = env.current_stats<AutoHostMultiGame_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 10);

    m_killswitch_time = WallClock::max();

    std::string report_name = "PokemonSV-AutoHostMultiGame-JoinReport-" + now_to_filestring() + ".txt";
    MultiLanguageJoinTracker join_tracker(1);

    TeraFailTracker fail_tracker(
        env, context,
        NOTIFICATION_ERROR_RECOVERABLE,
        CONSECUTIVE_FAILURE_PAUSE,
        FAILURE_PAUSE_MINUTES
    );
    KillSwitchTracker kill_switch(env);

    uint8_t g = 1;
    std::vector<std::unique_ptr<MultiHostSlot>> list = GAME_LIST.copy_snapshot();
    bool skip_reset = true;
    
    while (true){
        for (uint8_t index = 0; index < list.size(); index++){
            const MultiHostSlot& game = *list[index];
            env.update_stats();

            fail_tracker.on_raid_start();
          
            uint8_t game_slot = (uint8_t)game.game_slot.current_value();
            uint8_t game_user = (uint8_t)game.user_slot.current_value();
          
            if (!skip_reset) {
                pbf_press_button(context, BUTTON_HOME, 10, 125);
                pbf_press_button(context, BUTTON_X, 10, 125);
                pbf_press_button(context, BUTTON_A, 10, 1250);
            }
            skip_reset=false;
          
            if (game_slot == g) {
                pbf_press_button(context, BUTTON_A, 10, 250);
                while (game_user != 1) {
                    pbf_press_button(context, DPAD_RIGHT, 10, 20);
                    game_user--;
                }
                pbf_press_button(context, BUTTON_A, 10, 10);
                pbf_wait(context, 2500);
                pbf_press_button(context, BUTTON_A, 10, 125);
                pbf_press_button(context, BUTTON_A, 10, 125);
                pbf_press_button(context, BUTTON_A, 10, 125);
                pbf_wait(context, 2500);
            }
          
            if (game_slot != g) {
                pbf_press_button(context, DPAD_RIGHT, 10, 250);
                pbf_press_button(context, BUTTON_A, 10, 250);
                g = (g == 1) ? 2 : 1;
                while (game_user != 1) {
                    pbf_press_button(context, DPAD_RIGHT, 10, 20);
                    game_user--;
                }
                pbf_wait(context, 2500);
                pbf_press_button(context, BUTTON_A, 10, 125);
                pbf_press_button(context, BUTTON_A, 10, 125);
                pbf_press_button(context, BUTTON_A, 10, 125);
                pbf_wait(context, 2500);
            }
            pbf_wait(context, 625);

            //  Check kill-switch now before we go online.
            if (MODE != HostingMode::LOCAL){
                kill_switch.check_kill_switch(REMOTE_KILL_SWITCH0);
            }

            //  Store the mode locally in case the user changes in the middle of
            //  this iteration.
            HostingMode mode = MODE;

            if (mode != HostingMode::LOCAL){
                //  Connect to internet.
                try{
                    connect_to_internet_from_overworld(env.program_info(), env.console, context);
                }catch (OperationFailedException& e){
                    stats.m_errors++;
                    e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);
                    fail_tracker.report_raid_error();
                    continue;
                }
            }

            if (!open_raid(env.console, context)){
                env.log("No Tera raid found.", COLOR_RED);
                fail_tracker.report_raid_error();
                continue;
            }
            env.log("Tera raid found!", COLOR_BLUE);

            context.wait_for(std::chrono::milliseconds(100));
            
            BAN_LIST.refresh_online_table(env.logger());

            try{
                open_hosting_lobby(env, env.console, context, mode);
            }catch (OperationFailedException& e){
                stats.m_errors++;
                e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);
                fail_tracker.report_raid_error();
                continue;
            }
            
            try{
                std::string lobby_code;
                std::array<std::map<Language, std::string>, 4> player_names;

                if (!run_lobby(env, context, lobby_code, player_names)){
                    continue;
                }
                env.update_stats();

                stats.m_raids++;
                bool win = run_tera_battle(env, env.console, context, BATTLE_AI);
                env.update_stats();
                if (win){
                    stats.m_wins++;
                }else{
                    stats.m_losses++;
                }
                if (JOIN_REPORT.enabled() && (win || !JOIN_REPORT.wins_only)){
                    join_tracker.append(player_names, lobby_code);
                    join_tracker.dump(report_name);
                    send_program_notification_with_file(
                        env,
                        NOTIFICATION_JOIN_REPORT,
                        Color(0),
                        "Join Report",
                        {}, "",
                        report_name
                    );
                }
                if (win){
                    exit_tera_win_without_catching(env.program_info(), env.console, context, 0);
                }
                fail_tracker.report_successful_raid();
            }catch (OperationFailedException& e){
                stats.m_errors++;
                e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);
                fail_tracker.report_raid_error();
                continue;
            }
            pbf_wait(context, game.post_raid_delay);
        }
    }
}
}
}
}
