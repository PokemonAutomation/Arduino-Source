/*  Tera Multi-Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
//#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSV/PokemonSV_Settings.h"
//#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraRaidSearchDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_ConnectToInternet.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/FastCodeEntry/PokemonSV_CodeEntry.h"
#include "PokemonSV_TeraRoutines.h"
#include "PokemonSV_TeraBattler.h"
#include "PokemonSV_AutoHostTools.h"
#include "PokemonSV_AutoHostLobbyWaiter.h"
#include "PokemonSV_TeraMultiFarmer.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



GeneralHostingOptions::GeneralHostingOptions()
    : GroupOption("Hosting Options", LockMode::UNLOCK_WHILE_RUNNING)
{
    PA_ADD_OPTION(LOBBY_WAIT_DELAY);
    PA_ADD_OPTION(START_RAID_PLAYERS);
    PA_ADD_OPTION(SHOW_RAID_CODE);
    PA_ADD_OPTION(DESCRIPTION);
    PA_ADD_OPTION(REMOTE_KILL_SWITCH);
    PA_ADD_OPTION(CONSECUTIVE_FAILURE_PAUSE);
    PA_ADD_OPTION(FAILURE_PAUSE_MINUTES);
}




TeraFarmerPerConsoleOptions::~TeraFarmerPerConsoleOptions(){
    catch_on_win.remove_listener(*this);
}
TeraFarmerPerConsoleOptions::TeraFarmerPerConsoleOptions(std::string label, const LanguageSet& languages, bool host)
    : GroupOption(std::move(label), LockMode::UNLOCK_WHILE_RUNNING)
    , is_host_label("<font color=\"blue\" size=4><b>This is the host Switch.</b></font>")
    , language("<b>Game Language:</b>", languages, LockMode::LOCK_WHILE_RUNNING, true)
    , catch_on_win(
        "<b>Catch the " + STRING_POKEMON + ":</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , ball_select(
        "<b>Ball Select:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "poke-ball"
    )
{
    PA_ADD_STATIC(is_host_label);
    PA_ADD_OPTION(language);
    PA_ADD_OPTION(keyboard_layout);
    PA_ADD_OPTION(catch_on_win);
    PA_ADD_OPTION(ball_select);
    PA_ADD_OPTION(battle_ai);

    set_host(host);

    catch_on_win.add_listener(*this);
}
void TeraFarmerPerConsoleOptions::set_host(bool is_host){
    this->is_host = is_host;
    TeraFarmerPerConsoleOptions::on_config_value_changed(this);
}
void TeraFarmerPerConsoleOptions::on_config_value_changed(void* object){
    if (this->is_host){
        is_host_label.set_visibility(ConfigOptionState::ENABLED);
        catch_on_win.set_visibility(ConfigOptionState::DISABLED);
        ball_select.set_visibility(ConfigOptionState::DISABLED);
    }else{
        is_host_label.set_visibility(ConfigOptionState::HIDDEN);
        catch_on_win.set_visibility(ConfigOptionState::ENABLED);
        ball_select.set_visibility(catch_on_win ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED);
    }
}



TeraMultiFarmer_Descriptor::TeraMultiFarmer_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSV:TeraMultiFarmer",
        STRING_POKEMON + " SV", "Tera Multi-Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/TeraMultiFarmer.md",
        "Farm items and " + STRING_POKEMON + " from your own Tera raid using multiple Switches.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        2, 4, 2
    )
{}
struct TeraMultiFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_raids(m_stats["Raids"])
        , m_empty(m_stats["Empty Raids"])
        , m_full(m_stats["Full Raids"])
        , m_joiners(m_stats["Total Joiners"])
        , m_wins(m_stats["Wins"])
        , m_losses(m_stats["Losses"])
        , m_banned(m_stats["Banned"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Raids");
        m_display_order.emplace_back("Empty Raids", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Full Raids", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Total Joiners", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Banned", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_empty;
    std::atomic<uint64_t>& m_full;
    std::atomic<uint64_t>& m_joiners;
    std::atomic<uint64_t>& m_wins;
    std::atomic<uint64_t>& m_losses;
    std::atomic<uint64_t>& m_banned;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> TeraMultiFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


TeraMultiFarmer::~TeraMultiFarmer(){
    HOSTING_MODE.remove_listener(*this);
    HOSTING_SWITCH.remove_listener(*this);
}
TeraMultiFarmer::TeraMultiFarmer()
    : HOSTING_SWITCH(
        "<b>Host Switch:</b><br>This is the Switch that hosts the raid.",
        {
            {0, "switch0", "Switch 0 (Top Left)"},
            {1, "switch1", "Switch 1 (Top Right)"},
            {2, "switch2", "Switch 2 (Bottom Left)"},
            {3, "switch3", "Switch 3 (Bottom Right)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , MAX_WINS(
        "<b>Max Wins:</b><br>Stop program after winning this many times.",
          LockMode::UNLOCK_WHILE_RUNNING,
        999, 1, 999
    )
    , HOSTING_MODE(
        "<b>Mode:</b>",
        {
            {Mode::FARM_ALONE,      "farm-alone",   "Farm by yourself."},
            {Mode::HOST_LOCALLY,    "host-locally", "Host remaining slots locally."},
            {Mode::HOST_ONLINE,     "host-online",  "Host remaining slots online."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Mode::FARM_ALONE
    )
    , RECOVERY_MODE(
        "<b>Recovery Mode:</b>",
        {
            {RecoveryMode::STOP_ON_ERROR,   "stop-on-error",    "Stop on any error."},
            {RecoveryMode::SAVE_AND_RESET,  "save-and-reset",   "Save before each raid. Reset on errors."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        RecoveryMode::SAVE_AND_RESET
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_RAID_POST,
        &NOTIFICATION_RAID_START,
        &NOTIFICATION_JOIN_REPORT,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    const LanguageSet& languages = PokemonNameReader::instance().languages();
    size_t host = HOSTING_SWITCH.current_value();
    PLAYERS[0].reset(new TeraFarmerPerConsoleOptions("Switch 0 (Top Left)", languages, host == 0));
    PLAYERS[1].reset(new TeraFarmerPerConsoleOptions("Switch 1 (Top Right)", languages, host == 1));
    PLAYERS[2].reset(new TeraFarmerPerConsoleOptions("Switch 2 (Bottom Left)", languages, host == 2));
    PLAYERS[3].reset(new TeraFarmerPerConsoleOptions("Switch 3 (Bottom Right)", languages, host == 3));

    PA_ADD_OPTION(HOSTING_SWITCH);
    PA_ADD_OPTION(MAX_WINS);

    //  General Auto-Hosting Options
    PA_ADD_OPTION(HOSTING_MODE);
    PA_ADD_OPTION(HOSTING_OPTIONS);

    PA_ADD_OPTION(*PLAYERS[0]);
    PA_ADD_OPTION(*PLAYERS[1]);
    PA_ADD_OPTION(*PLAYERS[2]);
    PA_ADD_OPTION(*PLAYERS[3]);

    PA_ADD_OPTION(ROLLOVER_PREVENTION);
//    PA_ADD_OPTION(RECOVERY_MODE);

    //  Extended Auto-Hosting Options
    PA_ADD_OPTION(BAN_LIST);
    PA_ADD_OPTION(JOIN_REPORT);

    PA_ADD_OPTION(NOTIFICATIONS);

    TeraMultiFarmer::on_config_value_changed(this);

    HOSTING_SWITCH.add_listener(*this);
    HOSTING_MODE.add_listener(*this);
}
void TeraMultiFarmer::update_active_consoles(size_t switch_count){
    for (size_t c = 0; c < 4; c ++){
        PLAYERS[c]->set_visibility(c < switch_count ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    }
}
void TeraMultiFarmer::on_config_value_changed(void* object){
    size_t host = HOSTING_SWITCH.current_value();
    for (size_t c = 0; c < 4; c++){
        PLAYERS[c]->set_host(host == c);
    }

    ConfigOptionState hosting = HOSTING_MODE == Mode::FARM_ALONE
        ? ConfigOptionState::HIDDEN
        : ConfigOptionState::ENABLED;
    HOSTING_OPTIONS.set_visibility(hosting);
    BAN_LIST.set_visibility(hosting);
    JOIN_REPORT.set_visibility(hosting);
}


void TeraMultiFarmer::reset_host(const ProgramInfo& info, ConsoleHandle& console, ProControllerContext& context){
    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
    if (ROLLOVER_PREVENTION){
        WallClock now = current_time();
        if (m_last_time_fix == WallClock::min() || now - m_last_time_fix > std::chrono::hours(4)){
            set_time_to_12am_from_home(info, console, context);
            m_last_time_fix = now;
        }
    }
    reset_game_from_home(info, console, context, 5 * TICKS_PER_SECOND);
}
void TeraMultiFarmer::reset_joiner(const ProgramInfo& info, ConsoleHandle& console, ProControllerContext& context){
    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
    reset_game_from_home(info, console, context, 5 * TICKS_PER_SECOND);
}
bool TeraMultiFarmer::run_raid_host(ProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context){
    TeraMultiFarmer_Descriptor::Stats& stats = env.current_stats<TeraMultiFarmer_Descriptor::Stats>();
    TeraFarmerPerConsoleOptions& option = *PLAYERS[console.index()];

    stats.m_raids++;
    env.update_stats();
    bool win = run_tera_battle(env, console, context, option.battle_ai);

    if (win){
        stats.m_wins++;
        env.update_stats();
        if (HOSTING_MODE == Mode::HOST_ONLINE){
            exit_tera_win_without_catching(env.program_info(), console, context, 0);
        }
        reset_host(env.program_info(), console, context);
//        if (HOSTING_MODE == Mode::HOST_ONLINE){
//            connect_to_internet_from_overworld(env.program_info(), console, context);
//        }
    }else{
        stats.m_losses++;
        env.update_stats();
    }

//    open_raid(console, context);

    return win;
}
void TeraMultiFarmer::run_raid_joiner(ProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context){
    TeraFarmerPerConsoleOptions& option = *PLAYERS[console.index()];

    bool win = run_tera_battle(env, console, context, option.battle_ai);

    if (win){
        if (option.catch_on_win){
            exit_tera_win_by_catching(env, console, context, option.language, option.ball_select.slug(), 0);
        }else{
            exit_tera_win_without_catching(env.program_info(), console, context, 0);
        }
    }

    if (RECOVERY_MODE == RecoveryMode::SAVE_AND_RESET){
        pbf_press_button(context, BUTTON_X, 20, 105);
        save_game_from_menu(env.program_info(), console, context);
    }

//    enter_tera_search(env.program_info(), console, context, HOSTING_MODE == Mode::HOST_ONLINE);
}


bool TeraMultiFarmer::start_sequence_host(
    MultiSwitchProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context,
    RaidWaiter& raid_waiter, CancellableScope& joiner_scope,
    std::string& lobby_code,
    std::array<std::map<Language, std::string>, 4>& player_names
){
    TeraMultiFarmer_Descriptor::Stats& stats = env.current_stats<TeraMultiFarmer_Descriptor::Stats>();

    TeraCardReader card_detector(COLOR_RED);
    if (!card_detector.detect(console.video().snapshot())){
        if (HOSTING_MODE == Mode::HOST_ONLINE){
            connect_to_internet_from_overworld(env.program_info(), console, context);
        }
        open_raid(console, context);
    }

    if (HOSTING_MODE != Mode::FARM_ALONE){
        BAN_LIST.refresh_online_table(env.logger());
    }

    //  Open lobby and read code.
    WallClock lobby_start_time;
    try{
        TeraLobbyReader lobby_reader(console.logger());
        open_hosting_lobby(
            env, console, context,
            HOSTING_MODE == Mode::HOST_ONLINE
                ? HostingMode::ONLINE_CODED
                : HostingMode::LOCAL
        );
        lobby_start_time = current_time();
        std::string code = lobby_reader.raid_code(
            env.logger(),
            console.video().snapshot()
        );

//        code.back() = '0';

        const char* error = normalize_code(lobby_code, code);
        if (error){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to read raid code.",
                console
            );
        }
    }catch (OperationFailedException&){
        stats.m_errors++;
        m_reset_required[console.index()].store(true, std::memory_order_relaxed);
        throw;
    }

    raid_waiter.signal_raid_code_is_ready(lobby_code);
    raid_waiter.wait_for_joiners(env.consoles.size() - 1);

#if 1
    //  Put it up on auto-host.
    if (HOSTING_MODE != Mode::FARM_ALONE){
        send_host_announcement(
            env, console,
            lobby_code,
            HOSTING_OPTIONS.SHOW_RAID_CODE,
            HOSTING_OPTIONS.DESCRIPTION,
            NOTIFICATION_RAID_POST
        );

        TeraLobbyWaiter waiter(
            env, console, context,
            (uint8_t)env.consoles.size(),
            lobby_code, lobby_start_time,
            HOSTING_OPTIONS.LOBBY_WAIT_DELAY,
            HOSTING_OPTIONS.START_RAID_PLAYERS,
            NOTIFICATION_RAID_START,
            BAN_LIST,
            JOIN_REPORT
        );

        TeraLobbyWaiter::LobbyResult result = waiter.run_lobby();
        player_names = waiter.names();

        if (result == TeraLobbyWaiter::LobbyResult::BANNED_PLAYER){
            stats.m_banned++;
            m_reset_required[0].store(true, std::memory_order_relaxed);
            m_reset_required[1].store(true, std::memory_order_relaxed);
            m_reset_required[2].store(true, std::memory_order_relaxed);
            m_reset_required[3].store(true, std::memory_order_relaxed);
        }
        if (result != TeraLobbyWaiter::LobbyResult::RAID_STARTED){
            return false;
        }

        uint8_t hosts = (uint8_t)env.consoles.size();
        uint8_t players = waiter.last_known_players();
        if (players > hosts){
            stats.m_joiners += players - hosts;
        }
        if (players == 4){
            stats.m_full++;
        }else if (players == hosts){
            stats.m_empty++;
        }


    }
#endif

    //  Start the raid.
    pbf_mash_button(context, BUTTON_A, 10 * TICKS_PER_SECOND);

    return true;
}
void TeraMultiFarmer::start_sequence_joiner(
    ProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context,
    RaidWaiter& raid_waiter
){
    join_raid(
        env.program_info(), console, context,
        HOSTING_MODE == Mode::HOST_ONLINE,
        PLAYERS[console.index()]->keyboard_layout,
        raid_waiter
    );

    raid_waiter.signal_joiner_is_ready();

    //  Mash A to ready up. Even though this runs for 3 minutes, it will
    //  automatically be cancelled by the host when the host starts the raid.
    pbf_mash_button(context, BUTTON_A, std::chrono::minutes(3));
}

bool TeraMultiFarmer::run_raid(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    std::string& lobby_code,
    std::array<std::map<Language, std::string>, 4>& player_names
){
    TeraMultiFarmer_Descriptor::Stats& stats = env.current_stats<TeraMultiFarmer_Descriptor::Stats>();
    size_t host_index = HOSTING_SWITCH.current_value();

    CancellableHolder<RaidWaiter> raid_waiter(scope);
    CancellableHolder<CancellableScope> joiner_scope((CancellableScope&)raid_waiter);

    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        try{
            if (console.index() == host_index){
                start_sequence_host(
                    env, console, context,
                    raid_waiter,
                    joiner_scope,
                    lobby_code,
                    player_names
                );
                joiner_scope.cancel();
                return;
            }

            ProControllerContext sub_context(joiner_scope, context.controller());
            start_sequence_joiner(
                env, console, sub_context,
                raid_waiter
            );
            sub_context.wait_for_all_requests();

        }catch (OperationCancelledException&){
        }catch (OperationFailedException&){
            stats.m_errors++;
#if 0
            m_reset_required[console.index()].store(true, std::memory_order_relaxed);
#else
            m_reset_required[0].store(true, std::memory_order_relaxed);
            m_reset_required[1].store(true, std::memory_order_relaxed);
            m_reset_required[2].store(true, std::memory_order_relaxed);
            m_reset_required[3].store(true, std::memory_order_relaxed);
#endif
            raid_waiter.cancel();
//            cout << "OperationFailedException: " << console.index() << endl;
            throw;
        }catch (...){
            raid_waiter.cancel();
//            cout << "General Exception: " << console.index() << endl;
            throw;
        }
//        cout << "Finishing: " << console.index() << endl;
    });


    bool win = false;

    //  Run the raid.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        try{
            if (console.index() == host_index){
                win = run_raid_host(env, console, context);
            }else{
                run_raid_joiner(env, console, context);
            }
        }catch (OperationFailedException&){
            //  Host throws. Reset the host and keep going.
            stats.m_errors++;
            env.update_stats();
            m_reset_required[console.index()].store(true, std::memory_order_relaxed);
            throw;
        }
    });

    return win;
}
void TeraMultiFarmer::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    size_t host_index = HOSTING_SWITCH.current_value();
    if (host_index >= env.consoles.size()){
        throw UserSetupError(env.logger(), "The host Switch doesn't exist.");
    }
    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        assert_16_9_720p_min(console.logger(), console);
    });

//    Mode mode = MODE;
//    ConsoleHandle& host_console = env.consoles[host_index];
//    BotBaseContext host_context(scope, host_console.botbase());

    std::string report_name = "PokemonSV-AutoHost-JoinReport-" + now_to_filestring() + ".txt";
    MultiLanguageJoinTracker join_tracker((uint8_t)env.consoles.size());

    m_reset_required[0].store(false, std::memory_order_relaxed);
    m_reset_required[1].store(false, std::memory_order_relaxed);
    m_reset_required[2].store(false, std::memory_order_relaxed);
    m_reset_required[3].store(false, std::memory_order_relaxed);

    if (RECOVERY_MODE == RecoveryMode::SAVE_AND_RESET){
        env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
            if (console.index() != host_index){
                //  Do 2 presses in quick succession in case one drops or is
                //  needed to connect the controller.
                pbf_press_button(context, BUTTON_X, 5, 5);
                pbf_press_button(context, BUTTON_X, 20, 105);
                save_game_from_menu(env.program_info(), console, context);
            }else{
                pbf_press_button(context, BUTTON_L, 5, 5);
            }
        });
    }

    TeraFailTracker fail_tracker(
        env, scope,
        NOTIFICATION_ERROR_RECOVERABLE,
        HOSTING_OPTIONS.CONSECUTIVE_FAILURE_PAUSE,
        HOSTING_OPTIONS.FAILURE_PAUSE_MINUTES
    );
    KillSwitchTracker kill_switch(env);

    m_last_time_fix = WallClock::min();
    for (uint16_t wins = 0; wins < MAX_WINS;){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        //  We don't care about fail tracking if we're alone. No chance of ban.
        if (HOSTING_MODE != Mode::FARM_ALONE){
            fail_tracker.on_raid_start();
        }

        //  Reset all errored Switches.
        env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
            size_t index = console.index();
            if (!m_reset_required[index].load(std::memory_order_relaxed)){
                return;
            }
            if (index == host_index){
                reset_host(env.program_info(), console, context);
            }else{
                reset_joiner(env.program_info(), console, context);
            }
            m_reset_required[index].store(false, std::memory_order_relaxed);
        });

        //  Check kill-switch now before we go online.
        if (HOSTING_MODE == Mode::HOST_ONLINE){
            kill_switch.check_kill_switch(HOSTING_OPTIONS.REMOTE_KILL_SWITCH);
        }

        try{
            std::string lobby_code;
            std::array<std::map<Language, std::string>, 4> player_names;
            bool win = run_raid(env, scope, lobby_code, player_names);
            if (win){
                wins++;
            }
            if (HOSTING_MODE != Mode::FARM_ALONE && JOIN_REPORT.enabled() && (win || !JOIN_REPORT.wins_only)){
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
            fail_tracker.report_successful_raid();
        }catch (OperationFailedException& e){
//            cout << "caught: TeraMultiFarmer::program" << endl;

            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);
            if (RECOVERY_MODE != RecoveryMode::SAVE_AND_RESET){
                //  Iterate the errored Switches. If a non-host has errored,
                //  rethrow the exception to stop the program.
                for (size_t c = 0; c < 4; c++){
                    if (m_reset_required[c].load(std::memory_order_relaxed) && c != host_index){
                        throw;
                    }
                }
            }
            fail_tracker.report_raid_error();
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
