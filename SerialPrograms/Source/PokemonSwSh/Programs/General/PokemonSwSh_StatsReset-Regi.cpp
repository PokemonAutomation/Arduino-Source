/*  Stats Reset - Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "PokemonSwSh_StatsReset-Regi.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "CommonFramework/Tools/ProgramNotifications.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"

#include <QJsonArray>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StatsResetRegi_Descriptor::StatsResetRegi_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:StatsResetRegi",
        "Stats Reset - Regi",
        "SwSh-Arduino/wiki/Advanced:-StatsResetRegi",
        "Repeatedly catch regi until you get the stats you want.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



StatsResetRegi::StatsResetRegi(const StatsResetRegi_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , BALL_SELECT("<b>Ball Select:</b>", "master-ball")
    , LANGUAGE(
        "<b>Game Language:</b>",
        m_iv_checker_reader.languages(),
        true
    )
    , HP("<b>HP:</b>")
    , ATTACK("<b>Attack:</b>", 1)
    , DEFENSE("<b>Defense:</b>")
    , SPATK("<b>Sp. Atk:</b>")
    , SPDEF("<b>Sp. Def:</b>")
    , SPEED("<b>Speed:</b>")
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(HP);
    PA_ADD_OPTION(ATTACK);
    PA_ADD_OPTION(DEFENSE);
    PA_ADD_OPTION(SPATK);
    PA_ADD_OPTION(SPDEF);
    PA_ADD_OPTION(SPEED);
}



struct StatsResetRegi::Stats : public StatsTracker{
    Stats()
        : pokemon_caught(m_stats["Pokemon caught"])
        , pokemon_fainted(m_stats["Pokemon fainted"])
        , own_fainted(m_stats["Own fainted"])
        , out_of_balls(m_stats["Out of balls"])
        , errors(m_stats["Errors"])
        , total_balls_thrown(m_stats["Total balls thrown"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back(Stat("Pokemon caught"));
        m_display_order.emplace_back(Stat("Pokemon fainted"));
        m_display_order.emplace_back(Stat("Own fainted"));
        m_display_order.emplace_back(Stat("Out of balls"));
        m_display_order.emplace_back(Stat("Errors"));
        m_display_order.emplace_back(Stat("Total balls thrown"));
        m_display_order.emplace_back(Stat("Matches"));
    }

    uint64_t& pokemon_caught;
    uint64_t& pokemon_fainted;
    uint64_t& own_fainted;
    uint64_t& out_of_balls;
    uint64_t& errors;
    uint64_t& total_balls_thrown;
    uint64_t& matches;
};
std::unique_ptr<StatsTracker> StatsResetRegi::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



void StatsResetRegi::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    Stats& stats = env.stats<Stats>();

    bool match_found = false;
    while (!match_found){

        bool regi_caught = false;
        while (!regi_caught){
            env.log("Talk to regi.", "purple");
            env.console.botbase().wait_for_all_requests();
            {
                StandardBattleMenuDetector fight_detector(false);
                int result = run_until(
                    env, env.console,
                    [=](const BotBaseContext& context){
                        while (true){
                            pbf_press_button(context, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
                        }
                        context->wait_for_all_requests();
                    },
                    { &fight_detector }
                );
                if (result == 0){
                    env.log("New fight detected, let's begin to throw balls.", "purple");
                    pbf_mash_button(env.console, BUTTON_B, 1 * TICKS_PER_SECOND);
                }
            }

            env.log("Catch regi.", "purple");
            CatchResults result = basic_catcher(env, env.console, LANGUAGE, BALL_SELECT.slug());
            switch (result.result){
            case CatchResult::POKEMON_CAUGHT:
                regi_caught = true;
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
            QString message = "Threw " + QString::number(result.balls_used) + " balls " + (regi_caught ? "and caught it" : "and did not caught it");
//            DiscordWebHook::send_message_old(false, message, stats.make_discord_stats());
            send_program_status_notification(
                env.logger(), false,
                descriptor().display_name(),
                message,
                stats.to_str()
            );

            if (!regi_caught){
                pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
                reset_game_from_home_with_inference(
                    env, env.console,
                    TOLERATE_SYSTEM_UPDATE_MENU_SLOW
                );
            }
        }

        env.log("Check the stats.", "purple");
        for (int i = 0; i < 20; i++){
            pbf_press_button(env.console, BUTTON_B, 10, 1 * TICKS_PER_SECOND);
        }
        pbf_press_button(env.console, BUTTON_X  , 10, 2   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_RIGHT, 10, 0.5 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A  , 10, 2   * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_R  , 10, 3   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_LEFT , 10, 1   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_UP   , 10, 1   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_UP   , 10, 1   * TICKS_PER_SECOND);

        env.console.botbase().wait_for_all_requests();
        IVCheckerReaderScope reader(m_iv_checker_reader, env.console, LANGUAGE);
        IVCheckerReader::Results results = reader.read(env.console, env.console.video().snapshot());
        bool ok = true;
        ok &= HP.matches(stats.errors, results.hp);
        ok &= ATTACK.matches(stats.errors, results.attack);
        ok &= DEFENSE.matches(stats.errors, results.defense);
        ok &= SPATK.matches(stats.errors, results.spatk);
        ok &= SPDEF.matches(stats.errors, results.spdef);
        ok &= SPEED.matches(stats.errors, results.speed);
        env.update_stats();
        match_found = ok;

        if (!match_found){
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            reset_game_from_home_with_inference(
                env, env.console,
                TOLERATE_SYSTEM_UPDATE_MENU_SLOW
            );
        }
    }

//    DiscordWebHook::send_message_old(true, "Found a perfect match", stats.make_discord_stats());
    stats.matches++;
    env.update_stats();
    env.log("Result Found!", Qt::blue);
    send_program_finished_notification(
        env.logger(), true,
        descriptor().display_name(),
        "Found a perfect match!",
        stats.to_str()
    );

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
