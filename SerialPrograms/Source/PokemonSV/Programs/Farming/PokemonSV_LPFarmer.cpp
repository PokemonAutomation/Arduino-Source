/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV_LPFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


LPFarmer_Descriptor::LPFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:LPFarmer",
        STRING_POKEMON + " SV", "LP Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/LPFarmer.md",
        "Farm LP by day skipping Tera raids.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct LPFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_skips(m_stats["Day Skips"])
        , m_resets(m_stats["Resets"])
        , m_fetches(m_stats["Fetches"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Day Skips");
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Fetches");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_skips;
    std::atomic<uint64_t>& m_resets;
    std::atomic<uint64_t>& m_fetches;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> LPFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



LPFarmer::LPFarmer()
    : GO_HOME_WHEN_DONE(false)
    , FETCHES(
        "<b>Fetches:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        10000, 1
    )
    , PERIODIC_RESET(
        "<b>Periodic Game Reset:</b><br>Reset the game after this many skips. This clears up the framerate bug.",
        LockMode::UNLOCK_WHILE_RUNNING,
        20, 0, 100
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(FETCHES);
    PA_ADD_OPTION(PERIODIC_RESET);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void LPFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    LPFarmer_Descriptor::Stats& stats = env.current_stats<LPFarmer_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 10);

    bool first = true;
    uint32_t skip_counter = 0;

    for (size_t fetches = 0; fetches < FETCHES;){
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
        }

        if (!open_raid(env.console, context)){
            continue;
        }

        fetches++;
        stats.m_fetches++;

        close_raid(env.program_info(), env.console, context);
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}










}
}
}
