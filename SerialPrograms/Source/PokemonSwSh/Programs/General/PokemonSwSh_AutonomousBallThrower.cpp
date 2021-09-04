/*  Autonomous Ball Thrower
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh_AutonomousBallThrower.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "CommonFramework/Tools/DiscordWebHook.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"

#include <QJsonArray>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


AutonomousBallThrower_Descriptor::AutonomousBallThrower_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:AutonomousBallThrower",
        "Autonomous Ball Thrower",
        "SwSh-Arduino/wiki/Advanced:-AutonomousBallThrower",
        "Repeatedly throw a ball and reset until you catch the pokemon.",
        FeedbackType::REQUIRED,
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
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(LANGUAGE);
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

    uint64_t& pokemon_caught;
    uint64_t& pokemon_fainted;
    uint64_t& own_fainted;
    uint64_t& out_of_balls;
    uint64_t& errors;
    uint64_t& total_balls_thrown;
};
std::unique_ptr<StatsTracker> AutonomousBallThrower::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



void AutonomousBallThrower::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    Stats& stats = env.stats<Stats>();

    bool pokemon_caught = false;
    while (!pokemon_caught){
        env.console.botbase().wait_for_all_requests();
        env.log("Wait for a pokemon to attack you.", "purple");
        {
            InterruptableCommandSession commands(env.console);

            StandardBattleMenuDetector fight_detector(env.console, false);
            fight_detector.register_command_stop(commands);

            AsyncVisualInferenceSession inference(env, env.console);
            inference += fight_detector;

            commands.run([=](const BotBaseContext& context){
                while (true){
                    //TODO edit here for what to do
                    pbf_wait(context, 1 * TICKS_PER_SECOND);
                }
                context->wait_for_all_requests();
            });

            if (fight_detector.triggered()){
                env.log("New fight detected.", "purple");
                pbf_mash_button(env.console, BUTTON_B, 1 * TICKS_PER_SECOND);
            }
        }

        CatchResults result = basic_catcher(env, env.console, LANGUAGE, BALL_SELECT.slug());
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
        case CatchResult::TIMEOUT:
            stats.errors++;
            break;
        }
        stats.total_balls_thrown += result.balls_used;
        env.update_stats();
        QString message = "Threw " + QString::number(result.balls_used) + " balls " + (pokemon_caught ? "and caught it" : "and did not caught it");
        DiscordWebHook::send_message_old(false, message, stats.make_discord_stats());

        if (!pokemon_caught){
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            reset_game_from_home_with_inference(
                env, env.console,
                TOLERATE_SYSTEM_UPDATE_MENU_SLOW
            );
        }
    }

    DiscordWebHook::send_message_old(true, "Caught the pokemon", stats.make_discord_stats());
    env.log("Result Found!", Qt::blue);

    pbf_wait(env.console, 5 * TICKS_PER_SECOND);

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}


}
}
}
