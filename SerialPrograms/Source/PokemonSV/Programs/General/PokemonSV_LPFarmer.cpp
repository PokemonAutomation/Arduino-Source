/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
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
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct LPFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_skips(m_stats["Day Skips"])
        , m_fetches(m_stats["Fetches"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Day Skips");
        m_display_order.emplace_back("Fetches");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& m_skips;
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
        LockWhileRunning::UNLOCKED,
        10000, 1
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
    PA_ADD_OPTION(NOTIFICATIONS);
}



void LPFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    LPFarmer_Descriptor::Stats& stats = env.current_stats<LPFarmer_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 10);

    bool first = true;
    for (size_t fetches = 0; fetches < FETCHES;){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        if (!first){
            day_skip_from_overworld(env.console, context);
            pbf_wait(context, GameSettings::instance().RAID_SPAWN_DELAY);
            context.wait_for_all_requests();
            stats.m_skips++;
        }
        first = false;

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
