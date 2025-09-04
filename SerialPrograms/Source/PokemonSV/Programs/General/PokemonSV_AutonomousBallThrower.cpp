/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h"
#include "PokemonSV_AutonomousBallThrower.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


AutonomousBallThrower_Descriptor::AutonomousBallThrower_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:AutonomousBallThrower",
        STRING_POKEMON + " SV", "Autonomous Ball Thrower",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/AutonomousBallThrower.md",
        "Repeatedly throw a ball until you catch the pokemon.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct AutonomousBallThrower_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_balls(m_stats["Balls Thrown"])
    {
        m_display_order.emplace_back("Balls Thrown");
    }
    std::atomic<uint64_t>& m_balls;
};
std::unique_ptr<StatsTracker> AutonomousBallThrower_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



AutonomousBallThrower::AutonomousBallThrower()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages(),
        LockMode::UNLOCK_WHILE_RUNNING
    )
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "poke-ball"
    )
    , USE_FIRST_MOVE_IF_CANNOT_THROW_BALL(
        "<b>Use 1st Move if Cannot Throw Ball:</b><br>"
        "If you can't throw a ball because the opponent is semi-invulnerable, use the 1st move instead. "
        "Therefore, your first move should be non-damaging to avoid killing the wild " + STRING_POKEMON + ".",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , NOTIFICATION_CATCH_SUCCESS("Catch Success", true, true)
    , NOTIFICATION_CATCH_FAILED("Catch Failed", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_CATCH_SUCCESS,
        &NOTIFICATION_CATCH_FAILED,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(USE_FIRST_MOVE_IF_CANNOT_THROW_BALL);
    PA_ADD_OPTION(NOTIFICATIONS);
}




void AutonomousBallThrower::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    AutonomousBallThrower_Descriptor::Stats& stats = env.current_stats<AutonomousBallThrower_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    CatchResults results = basic_catcher(
        env.console, context, LANGUAGE,
        BALL_SELECT.slug(), 999,
        USE_FIRST_MOVE_IF_CANNOT_THROW_BALL,
        [&]{
            stats.m_balls++;
            env.update_stats();
        }
    );
    env.update_stats();

    send_catch_notification(
        env,
        NOTIFICATION_CATCH_SUCCESS,
        NOTIFICATION_CATCH_FAILED,
        nullptr,
        BALL_SELECT.slug(),
        results.balls_used,
        results.result
    );
}



}
}
}
