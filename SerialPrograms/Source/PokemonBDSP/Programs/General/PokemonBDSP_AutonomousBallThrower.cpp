/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BasicCatcher.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_AutonomousBallThrower.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


AutonomousBallThrower_Descriptor::AutonomousBallThrower_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSPh:AutonomousBallThrower",
        STRING_POKEMON + " BDSP", "Autonomous Ball Thrower",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/AutonomousBallThrower.md",
        "Repeatedly throw a ball and reset until you catch the pokemon.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct AutonomousBallThrower_Descriptor::Stats : public StatsTracker{
    Stats()
        : pokemon_caught(m_stats["Pokemon caught"])
        , pokemon_fainted(m_stats["Pokemon fainted"])
        , own_fainted(m_stats["Own fainted"])
        , out_of_balls(m_stats["Out of balls"])
        , errors(m_stats["Errors"])
        , total_balls_thrown(m_stats["Total balls thrown"])
    {
        m_display_order.emplace_back(Stat("Pokemon caught"));
        m_display_order.emplace_back(Stat("Pokemon fainted"));
        m_display_order.emplace_back(Stat("Own fainted"));
        m_display_order.emplace_back(Stat("Out of balls"));
        m_display_order.emplace_back(Stat("Errors"));
        m_display_order.emplace_back(Stat("Total balls thrown"));
    }

    std::atomic<uint64_t>& pokemon_caught;
    std::atomic<uint64_t>& pokemon_fainted;
    std::atomic<uint64_t>& own_fainted;
    std::atomic<uint64_t>& out_of_balls;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& total_balls_thrown;
};
std::unique_ptr<StatsTracker> AutonomousBallThrower_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



AutonomousBallThrower::AutonomousBallThrower()
    : GO_HOME_WHEN_DONE(false)
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "master-ball"
    )
    , LANGUAGE(
        "<b>Game Language:</b>",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
            Language::Korean,
            Language::ChineseSimplified,
            Language::ChineseTraditional,
        },
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , NOTIFICATION_CATCH_SUCCESS("Catch Success", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_CATCH_FAILED("Catch Failed", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_CATCH_SUCCESS,
        &NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(NOTIFICATIONS);
}




void AutonomousBallThrower::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AutonomousBallThrower_Descriptor::Stats& stats = env.current_stats<AutonomousBallThrower_Descriptor::Stats>();
    env.update_stats();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    bool pokemon_caught = false;
    while (!pokemon_caught){
        context.wait_for_all_requests();
        env.log("Wait for a pokemon to attack you.", COLOR_PURPLE);
        {
            BattleMenuWatcher fight_detector(BattleType::STANDARD);
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    while (true){
                        //TODO edit here for what to do
                        //pbf_wait(context, 1 * TICKS_PER_SECOND);
                        pbf_press_button(context, BUTTON_A, 5, TICKS_PER_SECOND);
                        pbf_press_dpad(context, DPAD_UP, 5, TICKS_PER_SECOND);
                    }
                },
                {{fight_detector}}
            );
            if (ret == 0){
                env.log("New fight detected.", COLOR_PURPLE);
                pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);
            }
        }

        CatchResults result = basic_catcher(env.console, context, LANGUAGE, BALL_SELECT.slug(), 999);
        switch (result.result){
        case CatchResult::POKEMON_CAUGHT:
            pokemon_caught = true;
            stats.pokemon_caught++;
            break;
        case CatchResult::POKEMON_FAINTED:
            stats.pokemon_fainted++;
            break;
        case CatchResult::OWN_FAINTED:
            stats.own_fainted++;
            break;
        case CatchResult::OUT_OF_BALLS:
            stats.out_of_balls++;
            break;
        case CatchResult::BALL_LIMIT_REACHED:
        case CatchResult::CANNOT_THROW_BALL:
        case CatchResult::TIMED_OUT:
            stats.errors++;
            break;
        }
        stats.total_balls_thrown += result.balls_used;
        env.update_stats();

        if (pokemon_caught){
            send_program_status_notification(
                env, NOTIFICATION_CATCH_SUCCESS,
                "Threw " + std::to_string(result.balls_used) + " ball(s) and caught it."
            );
        }else{
            send_program_status_notification(
                env, NOTIFICATION_CATCH_FAILED,
                "Threw " + std::to_string(result.balls_used) + " ball(s) and did not catch it."
            );
        }

        if (!pokemon_caught){
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
            reset_game_from_home(env, env.console, context, true);
        }
    }

    env.log("Result Found!", COLOR_BLUE);
    send_program_finished_notification(
        env, NOTIFICATION_PROGRAM_FINISH,
        "Caught the " + STRING_POKEMON
    );
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


}
}
}
