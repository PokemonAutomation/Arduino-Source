/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BasicCatcher.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_AutonomousBallThrower.h"

#include <QJsonArray>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


AutonomousBallThrower_Descriptor::AutonomousBallThrower_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSPh:AutonomousBallThrower",
        STRING_POKEMON + " BDSP", "Autonomous Ball Thrower",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/AutonomousBallThrower.md",
        "Repeatedly throw a ball and reset until you catch the pokemon.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



AutonomousBallThrower::AutonomousBallThrower(const AutonomousBallThrower_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , BALL_SELECT("<b>Ball Select:</b>", "master-ball")
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



struct AutonomousBallThrower::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> AutonomousBallThrower::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



void AutonomousBallThrower::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();
    env.update_stats();

    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_LCLICK, 5, 5);

    bool pokemon_caught = false;
    while (!pokemon_caught){
        env.console.botbase().wait_for_all_requests();
        env.log("Wait for a pokemon to attack you.", COLOR_PURPLE);
        {
            BattleMenuWatcher fight_detector(BattleType::STANDARD);
            int result = run_until(
                env, context, env.console,
                [=](BotBaseContext& context){
                    while (true){
                        //TODO edit here for what to do
                        //pbf_wait(context, 1 * TICKS_PER_SECOND);
                        pbf_press_button(context, BUTTON_A, 5, TICKS_PER_SECOND);
                        pbf_press_dpad(context, DPAD_UP, 5, TICKS_PER_SECOND);
                    }
                },
                { &fight_detector }
            );
            if (result == 0){
                env.log("New fight detected.", COLOR_PURPLE);
                pbf_mash_button(env.console, BUTTON_B, 1 * TICKS_PER_SECOND);
            }
        }

        CatchResults result = basic_catcher(env, context, env.console, LANGUAGE, BALL_SELECT.slug());
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
        case CatchResult::CANNOT_THROW_BALL:
        case CatchResult::TIMEOUT:
            stats.errors++;
            break;
        }
        stats.total_balls_thrown += result.balls_used;
        env.update_stats();

        if (pokemon_caught){
            send_program_status_notification(
                env.logger(), NOTIFICATION_CATCH_SUCCESS,
                env.program_info(),
                "Threw " + QString::number(result.balls_used) + " ball(s) and caught it.",
                stats.to_str()
            );
        }else{
            send_program_status_notification(
                env.logger(), NOTIFICATION_CATCH_FAILED,
                env.program_info(),
                "Threw " + QString::number(result.balls_used) + " ball(s) and did not catch it.",
                stats.to_str()
            );
        }

        if (!pokemon_caught){
            pbf_press_button(env.console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, context, env.console, true);
        }
    }

    env.log("Result Found!", COLOR_BLUE);

    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "Caught the " + STRING_POKEMON,
        stats.to_str()
    );
    GO_HOME_WHEN_DONE.run_end_of_program(env.console);
}


}
}
}
