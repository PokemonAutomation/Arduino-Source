/*  Tera Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraBattler.h"
#include "PokemonSV_TeraSelfFarmer.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


TeraSelfFarmer_Descriptor::TeraSelfFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:TeraSelfFarmer",
        STRING_POKEMON + " SV", "Tera Self Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/TeraSelfFarmer.md",
        "Farm items and " + STRING_POKEMON + " from Tera raids. Can also hunt for shiny and high reward raids.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct TeraSelfFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_skips(m_stats["Date Skips"])
        , m_raids(m_stats["Raids"])
        , m_wins(m_stats["Wins"])
        , m_losses(m_stats["Losses"])
        , m_skipped(m_stats["Skipped"])
        , m_errors(m_stats["Errors"])
        , m_caught(m_stats["Caught"])
        , m_shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Date Skips");
        m_display_order.emplace_back("Raids");
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Skipped");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Caught", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_skips;
    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_wins;
    std::atomic<uint64_t>& m_losses;
    std::atomic<uint64_t>& m_skipped;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_caught;
    std::atomic<uint64_t>& m_shinies;
};
std::unique_ptr<StatsTracker> TeraSelfFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



TeraFarmerStopConditions::TeraFarmerStopConditions()
    : GroupOption("Stop Conditions", LockMode::UNLOCK_WHILE_RUNNING)
    , MAX_CATCHES(
        "<b>Max Catches:</b><br>Stop program after catching this many " + STRING_POKEMON + ".",
        LockMode::UNLOCK_WHILE_RUNNING,
        50, 1, 999
    )
    , STOP_ON_SHINY(
        "<b>Stop on Shiny:</b> (requires catching the " + STRING_POKEMON + ")<br>"
        "Stop the program if a shiny is found. Resetting the game will return you to the front of this (shiny) raid so it can be hosted again.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , STOP_ON_RARE_ITEMS(
        "<b>Stop on Rare Items:</b><br>"
        "Stop the program if the rewards contain at least this many rare (sparkly) items. Set to zero to disable this feature and never stop for item rewards.<br>"
        "Note that the program can only see the first 8 item rewards. It will not scroll down.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0, 0, 8
    )
{
    PA_ADD_OPTION(MAX_CATCHES);
    PA_ADD_OPTION(STOP_ON_SHINY);
    PA_ADD_OPTION(STOP_ON_RARE_ITEMS);
}


TeraSelfFarmer::~TeraSelfFarmer(){
    CATCH_ON_WIN.remove_listener(*this);
}
TeraSelfFarmer::TeraSelfFarmer()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages(),
        LockMode::UNLOCK_WHILE_RUNNING
    )
    , FILTER(4, true)
    , PERIODIC_RESET(
        "<b>Periodic Game Reset:</b><br>Reset the game after this many skips. This clears up the framerate bug.",
        LockMode::UNLOCK_WHILE_RUNNING,
        20, 0, 100
    )
    , CATCH_ON_WIN(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
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
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_NONSHINY,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(FILTER);
    PA_ADD_OPTION(BATTLE_AI);
    PA_ADD_OPTION(PERIODIC_RESET);
    PA_ADD_OPTION(CATCH_ON_WIN);
    PA_ADD_OPTION(STOP_CONDITIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    CATCH_ON_WIN.add_listener(*this);
}
void TeraSelfFarmer::on_config_value_changed(void* object){
    STOP_CONDITIONS.STOP_ON_SHINY.set_visibility(
        CATCH_ON_WIN.enabled() ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );
}


bool TeraSelfFarmer::run_raid(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.console.log("Running raid...");

    TeraSelfFarmer_Descriptor::Stats& stats = env.current_stats<TeraSelfFarmer_Descriptor::Stats>();

    bool win = run_tera_battle(env, env.console, context, BATTLE_AI);

    if (win){
        stats.m_wins++;
    }else{
        stats.m_losses++;
        context.wait_for(std::chrono::seconds(3));
        return false;
    }

    if (!CATCH_ON_WIN.enabled()){
        exit_tera_win_without_catching(env.program_info(), env.console, context, STOP_CONDITIONS.STOP_ON_RARE_ITEMS);
        return true;
    }

    VideoSnapshot battle_snapshot = env.console.video().snapshot();


    if (CATCH_ON_WIN.FIX_TIME_ON_CATCH){
        go_home(env.console, context);
        home_to_date_time(env.console, context, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(env.console, context);
    }

    m_number_caught++;
    stats.m_caught++;

    exit_tera_win_by_catching(
        env, env.console, context,
        LANGUAGE,
        CATCH_ON_WIN.BALL_SELECT.slug(),
        NOTIFICATION_NONSHINY,
        NOTIFICATION_SHINY,
        STOP_CONDITIONS.STOP_ON_SHINY,
        STOP_CONDITIONS.STOP_ON_RARE_ITEMS,
        &stats.m_shinies
    );
    return true;
}


void TeraSelfFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    TeraSelfFarmer_Descriptor::Stats& stats = env.current_stats<TeraSelfFarmer_Descriptor::Stats>();

    if (FILTER.MIN_STARS > FILTER.MAX_STARS){
        throw UserSetupError(env.console, "Error in the settings, \"Min Stars\" is bigger than \"Max Stars\".");
    }

    if (FILTER.SKIP_NON_HERBA && FILTER.MAX_STARS < 5){
        throw UserSetupError(env.console, "Error in the settings, Skip Non-Herba Raids is checked but Max Stars is less than 5.");
    }

    m_number_caught = 0;

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 10);

    bool first = true;
    uint32_t skip_counter = 0;

    while (true){
        if (m_number_caught >= STOP_CONDITIONS.MAX_CATCHES){
            break;
        }

        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        if (!first){
            day_skip_from_overworld(env.console, context);
            pbf_wait(context, GameSettings::instance().RAID_SPAWN_DELAY0);
            context.wait_for_all_requests();
            stats.m_skips++;
            skip_counter++;
            env.update_stats();
        }
        first = false;

        uint8_t reset_period = PERIODIC_RESET;
        if (reset_period != 0 && skip_counter >= reset_period){
            env.log("Resetting game to clear framerate.");
            save_game_from_overworld(env.program_info(), env.console, context);
            reset_game(env.program_info(), env.console, context);
            skip_counter = 0;
//            stats.m_resets++;
        }

        TeraRaidData raid_data;
        TeraRollFilter::FilterResult result = FILTER.run_filter(
            env.program_info(), env.console, context,
            raid_data
        );
        switch (result){
        case TeraRollFilter::FilterResult::NO_RAID:
            continue;
        case TeraRollFilter::FilterResult::FAILED:
            stats.m_raids++;
            stats.m_skipped++;
            continue;
        case TeraRollFilter::FilterResult::PASSED:
            stats.m_raids++;
            break;
        }
        

        close_raid(env.program_info(), env.console, context);
        save_game_from_overworld(env.program_info(), env.console, context);

        context.wait_for_all_requests();
        if (open_raid(env.console, context)){
            env.log("Tera raid found!", COLOR_BLUE);
        }else{
            env.log("No Tera raid found.", COLOR_ORANGE);
            continue;
        }

        pbf_press_dpad(context, DPAD_DOWN, 10, 10);
        pbf_mash_button(context, BUTTON_A, 250);
        bool raid_won = run_raid(env, context);
        {
            std::stringstream ss;
            ss << "You ";
            if (raid_won){
                ss << "won";
            }else{
                ss << "lost";
            }

            std::string stars = raid_data.stars == 0
                ? "?"
                : std::to_string(raid_data.stars);
            std::string tera_type = raid_data.tera_type.empty()
                ? "unknown tera type"
                : raid_data.tera_type;

            std::string pokemon;
            if (raid_data.species.empty()){
                pokemon = "unknown " + Pokemon::STRING_POKEMON;
            }else if (raid_data.species.size() == 1){
                pokemon = *raid_data.species.begin();
            }else{
                pokemon = set_to_str(raid_data.species);
            }

            ss << " a " << stars << "* " << tera_type << " " << pokemon << " raid";
            env.log(ss.str());
            env.console.overlay().add_log(ss.str(), COLOR_GREEN);
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}











}
}
}
