/*  Tera Multi-Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_ConnectToInternet.h"
#include "PokemonSV/Programs/PokemonSV_CodeEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraBattler.h"
#include "PokemonSV_TeraMultiFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



PerConsoleTeraFarmerOptions::~PerConsoleTeraFarmerOptions(){
    catch_on_win.remove_listener(*this);
}
PerConsoleTeraFarmerOptions::PerConsoleTeraFarmerOptions(std::string label, const LanguageSet& languages, bool host)
    : GroupOption(std::move(label), LockWhileRunning::UNLOCKED)
    , is_host_label("<font color=\"blue\" size=4><b>This is the host Switch.</b></font>")
    , language("<b>Game Language:</b>", languages, LockWhileRunning::LOCKED, true)
    , catch_on_win(
        "<b>Catch the " + STRING_POKEMON + ":</b>",
        LockWhileRunning::UNLOCKED,
        false
    )
    , ball_select(
        "<b>Ball Select:</b>",
        LockWhileRunning::UNLOCKED,
        "poke-ball"
    )
{
    PA_ADD_STATIC(is_host_label);
    PA_ADD_OPTION(language);
    PA_ADD_OPTION(catch_on_win);
    PA_ADD_OPTION(ball_select);
    PA_ADD_OPTION(battle_ai);

    set_host(host);

    catch_on_win.add_listener(*this);
}
void PerConsoleTeraFarmerOptions::set_host(bool is_host){
    this->is_host = is_host;
    PerConsoleTeraFarmerOptions::value_changed();
}
void PerConsoleTeraFarmerOptions::value_changed(){
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
        "Farm items and " + STRING_POKEMON + " from a Tera raid using multiple Switches.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB,
        2, 4, 2
    )
{}
struct TeraMultiFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_raids(m_stats["Raids"])
        , m_wins(m_stats["Wins"])
        , m_losses(m_stats["Losses"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Raids");
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_wins;
    std::atomic<uint64_t>& m_losses;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> TeraMultiFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


TeraMultiFarmer::~TeraMultiFarmer(){
    HOSTING_SWITCH.remove_listener(*this);
}
TeraMultiFarmer::TeraMultiFarmer()
    : MODE(
        "<b>Mode:</b>",
        {
            {Mode::FARM_ALONE,      "farm-alone",   "Farm by yourself."},
            {Mode::HOST_LOCALLY,    "host-locally", "Host empty slots locally."},
            {Mode::HOST_ONLINE,     "host-online",  "Host empty slots online."},
        },
        LockWhileRunning::LOCKED,
        Mode::FARM_ALONE
    )
    , HOSTING_SWITCH(
        "<b>Host Switch:</b><br>This is the Switch that hosts the raid.",
        {
            {0, "switch0", "Switch 0 (Top Left)"},
            {1, "switch1", "Switch 1 (Top Right)"},
            {2, "switch2", "Switch 2 (Bottom Left)"},
            {3, "switch3", "Switch 3 (Bottom Right)"},
        },
        LockWhileRunning::LOCKED,
        0
    )
    , MAX_WINS(
        "<b>Max Wins:</b><br>Stop program after winning this many times.",
        LockWhileRunning::UNLOCKED,
        999, 1, 999
    )
    , ROLLOVER_PREVENTION(
        "<b>Rollover Prevention:</b><br>Periodically set the time back to 12AM to prevent the date from rolling over and losing the raid.",
        LockWhileRunning::UNLOCKED,
        true
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    const LanguageSet& languages = PokemonNameReader::instance().languages();
    size_t host = HOSTING_SWITCH.current_value();
    PLAYERS[0].reset(new PerConsoleTeraFarmerOptions("Switch 0 (Top Left)", languages, host == 0));
    PLAYERS[1].reset(new PerConsoleTeraFarmerOptions("Switch 1 (Top Right)", languages, host == 1));
    PLAYERS[2].reset(new PerConsoleTeraFarmerOptions("Switch 2 (Bottom Left)", languages, host == 2));
    PLAYERS[3].reset(new PerConsoleTeraFarmerOptions("Switch 3 (Bottom Right)", languages, host == 3));

//    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(HOSTING_SWITCH);
    PA_ADD_OPTION(MAX_WINS);
    PA_ADD_OPTION(*PLAYERS[0]);
    PA_ADD_OPTION(*PLAYERS[1]);
    PA_ADD_OPTION(*PLAYERS[2]);
    PA_ADD_OPTION(*PLAYERS[3]);

    PA_ADD_OPTION(ROLLOVER_PREVENTION);
    PA_ADD_OPTION(NOTIFICATIONS);

    TeraMultiFarmer::value_changed();

    HOSTING_SWITCH.add_listener(*this);
}
void TeraMultiFarmer::update_active_consoles(size_t switch_count){
    for (size_t c = 0; c < 4; c ++){
        PLAYERS[c]->set_visibility(c < switch_count ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    }
}
void TeraMultiFarmer::value_changed(){
    size_t host = HOSTING_SWITCH.current_value();
    for (size_t c = 0; c < 4; c++){
        PLAYERS[c]->set_host(host == c);
    }
}


void TeraMultiFarmer::reset_host(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    if (ROLLOVER_PREVENTION){
        WallClock now = current_time();
        if (m_last_time_fix == WallClock::min() || now - m_last_time_fix > std::chrono::hours(4)){
            set_time_to_12am_from_home(info, console, context);
            m_last_time_fix = now;
        }
    }
    reset_game_from_home(info, console, context, 5 * TICKS_PER_SECOND);
}
void TeraMultiFarmer::reset_joiner(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    reset_game_from_home(info, console, context, 5 * TICKS_PER_SECOND);
}
void TeraMultiFarmer::run_raid_host(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    TeraMultiFarmer_Descriptor::Stats& stats = env.current_stats<TeraMultiFarmer_Descriptor::Stats>();
    PerConsoleTeraFarmerOptions& option = *PLAYERS[console.index()];

    stats.m_raids++;
    env.update_stats();
    bool win = run_tera_battle(
        env, console, context,
        NOTIFICATION_ERROR_RECOVERABLE,
        option.battle_ai
    );

    if (win){
        stats.m_wins++;
        env.update_stats();
        if (MODE == Mode::HOST_ONLINE){
            exit_tera_win_without_catching(env.program_info(), console, context);
        }
        reset_host(env.program_info(), console, context);
        if (MODE == Mode::HOST_ONLINE){
            connect_to_internet_from_overworld(env.program_info(), console, context);
        }
    }else{
        stats.m_losses++;
        env.update_stats();
    }

    open_raid(console, context);
}
void TeraMultiFarmer::run_raid_joiner(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    PerConsoleTeraFarmerOptions& option = *PLAYERS[console.index()];

    bool win = run_tera_battle(
        env, console, context,
        NOTIFICATION_ERROR_RECOVERABLE,
        option.battle_ai
    );

    if (win){
        if (option.catch_on_win){
            exit_tera_win_by_catching(env, console, context, option.language, option.ball_select.slug());
        }else{
            exit_tera_win_without_catching(env.program_info(), console, context);
        }
    }

    enter_tera_search(env.program_info(), console, context, MODE == Mode::HOST_ONLINE);
}
bool TeraMultiFarmer::run_raid(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
//    TeraMultiFarmer_Descriptor::Stats& stats = env.current_stats<TeraMultiFarmer_Descriptor::Stats>();
    size_t host_index = HOSTING_SWITCH.current_value();
    ConsoleHandle& host_console = env.consoles[host_index];
    BotBaseContext host_context(scope, host_console.botbase());

    //  Open lobby and read code.
    TeraLobbyReader lobby_reader(host_console.logger(), env.realtime_dispatcher());
    open_hosting_lobby(
        env, host_console, host_context,
        MODE == Mode::HOST_ONLINE
            ? HostingMode::ONLINE_CODED
            : HostingMode::LOCAL
    );
    std::string code = lobby_reader.raid_code(env.logger(), env.realtime_dispatcher(), host_console.video().snapshot());
    std::string normalized_code;
    const char* error = normalize_code(normalized_code, code);
    if (error){
//        pbf_press_button(host_context, BUTTON_B, 20, 230);
//        pbf_press_button(host_context, BUTTON_A, 20, 230);
        throw OperationFailedException(env.logger(), "Unable to read raid code.");
    }

    //  Join the lobby.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        if (console.index() == host_index){
            return;
        }

        enter_code(console, context, FastCodeEntrySettings(), normalized_code, false);

        TeraLobbyWatcher lobby(console.logger(), env.realtime_dispatcher());
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context, std::chrono::seconds(60),
            {{lobby, std::chrono::milliseconds(500)}}
        );
        if (ret < 0){
            throw OperationFailedException(console.logger(), "Unable to join lobby.");
        }

        pbf_mash_button(context, BUTTON_A, 125);
    });

    if (MODE != Mode::FARM_ALONE){

    }

    //  Start the raid.
    pbf_mash_button(host_context, BUTTON_A, 10 * TICKS_PER_SECOND);


    bool global_win = false;
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        if (console.index() == host_index){
            run_raid_host(env, console, context);
        }else{
            run_raid_joiner(env, console, context);
        }
    });

    return global_win;
}
void TeraMultiFarmer::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    size_t host_index = HOSTING_SWITCH.current_value();
    if (host_index >= env.consoles.size()){
        throw UserSetupError(env.logger(), "The host Switch doesn't exist.");
    }
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        assert_16_9_720p_min(console.logger(), console);
    });

//    Mode mode = MODE;
//    ConsoleHandle& host_console = env.consoles[host_index];
//    BotBaseContext host_context(scope, host_console.botbase());

    //  Get everyone ready.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        if (console.index() == host_index){
            if (MODE == Mode::HOST_ONLINE){
                connect_to_internet_from_overworld(env.program_info(), console, context);
            }
            open_raid(console, context);
        }else{
            enter_tera_search(env.program_info(), console, context, MODE == Mode::HOST_ONLINE);
        }
    });

    m_last_time_fix = WallClock::min();
    for (uint16_t wins = 0; wins < MAX_WINS;){
        if (run_raid(env, scope)){
            wins++;
        }
    }


}




}
}
}
