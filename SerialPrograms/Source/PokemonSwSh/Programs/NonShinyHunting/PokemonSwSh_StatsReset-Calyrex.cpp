/*  Stats Reset - Calyrex
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh_StatsReset-Calyrex.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"

#include <QJsonArray>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StatsResetCalyrex_Descriptor::StatsResetCalyrex_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:StatsResetCalyrex",
        STRING_POKEMON + " SwSh", "Stats Reset - Calyrex",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/StatsReset-Calyrex.md",
        "Repeatedly catch calyrex (and its horse) until you get the stats you want.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



StatsResetCalyrex::StatsResetCalyrex(const StatsResetCalyrex_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , BALL_SELECT("<b>Ball Select:</b>", "master-ball")
    , LANGUAGE(
        "<b>Game Language:</b>",
        m_iv_checker_reader.languages(),
        true
    )
    , CHECK_CALYREX_STATS(
        "<b>Check Calyrex stats</b>",
        true
    )
    , CALYREX_HP("<b>Calyrex HP:</b>")
    , CALYREX_ATTACK("<b>Calyrex Attack:</b>", 1)
    , CALYREX_DEFENSE("<b>Calyrex Defense:</b>")
    , CALYREX_SPATK("<b>Calyrex Sp. Atk:</b>")
    , CALYREX_SPDEF("<b>Calyrex Sp. Def:</b>")
    , CALYREX_SPEED("<b>Calyrex Speed:</b>")
    , CHECK_HORSE_STATS(
        "<b>Check Horse stats</b>",
        true
    )
    , HORSE_HP("<b>Horse HP:</b>")
    , HORSE_ATTACK("<b>Horse Attack:</b>", 1)
    , HORSE_DEFENSE("<b>Horse Defense:</b>")
    , HORSE_SPATK("<b>Horse Sp. Atk:</b>")
    , HORSE_SPDEF("<b>Horse Sp. Def:</b>")
    , HORSE_SPEED("<b>Horse Speed:</b>")
    , NOTIFICATION_CATCH_SUCCESS("Catch Success", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_CATCH_FAILED("Catch Failed", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_CATCH_SUCCESS,
        &NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_PROGRAM_ERROR,
    })
{

    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(LANGUAGE);

    PA_ADD_OPTION(CHECK_CALYREX_STATS);
    PA_ADD_OPTION(CALYREX_HP);
    PA_ADD_OPTION(CALYREX_ATTACK);
    PA_ADD_OPTION(CALYREX_DEFENSE);
    PA_ADD_OPTION(CALYREX_SPATK);
    PA_ADD_OPTION(CALYREX_SPDEF);
    PA_ADD_OPTION(CALYREX_SPEED);

    PA_ADD_OPTION(CHECK_HORSE_STATS);
    PA_ADD_OPTION(HORSE_HP);
    PA_ADD_OPTION(HORSE_ATTACK);
    PA_ADD_OPTION(HORSE_DEFENSE);
    PA_ADD_OPTION(HORSE_SPATK);
    PA_ADD_OPTION(HORSE_SPDEF);
    PA_ADD_OPTION(HORSE_SPEED);

    PA_ADD_OPTION(NOTIFICATIONS);
}



struct StatsResetCalyrex::Stats : public StatsTracker{
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

    std::atomic<uint64_t>& pokemon_caught;
    std::atomic<uint64_t>& pokemon_fainted;
    std::atomic<uint64_t>& own_fainted;
    std::atomic<uint64_t>& out_of_balls;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& total_balls_thrown;
    std::atomic<uint64_t>& matches;
};
std::unique_ptr<StatsTracker> StatsResetCalyrex::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



void StatsResetCalyrex::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    Stats& stats = env.stats<Stats>();

    bool match_found = false;
    while (!match_found){

        bool calyrex_caught = false;
        while (!calyrex_caught){
            env.log("Talk to calyrex.", "purple");
            env.console.botbase().wait_for_all_requests();
            {
                StandardBattleMenuDetector fight_detector(false);
                int result = run_until(
                    env, env.console,
                    [=](const BotBaseContext& context){
                    while (true){
                        pbf_press_button(context, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
                    }
                },
                { &fight_detector }
                );
                if (result == 0) {
                    env.log("New fight detected, let's begin to throw balls.", "purple");
                    pbf_mash_button(env.console, BUTTON_B, 1 * TICKS_PER_SECOND);
                }
            }

            pbf_mash_button(env.console, BUTTON_B, 1 * TICKS_PER_SECOND);
            CatchResults result = basic_catcher(env, env.console, LANGUAGE, BALL_SELECT.slug());
            switch (result.result){
            case CatchResult::POKEMON_CAUGHT:
                calyrex_caught = true;
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

            if (calyrex_caught){
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

            if (!calyrex_caught){
                pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
                reset_game_from_home_with_inference(
                    env, env.console,
                    ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
                );
            }
        }

        env.log("Unfuse calyrex.", "purple");
        for (int i = 0; i < 40; i++){
            pbf_press_button(env.console, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
        }
        pbf_press_button(env.console, BUTTON_X  , 10, 1.5 * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_RIGHT, 10, 0.5 * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_RIGHT, 10, 0.5 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A  , 10, 2   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_LEFT , 10, 0.5 * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_UP   , 10, 0.5 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A  , 10, 2   * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A  , 10, 2   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_UP   , 10, 2   * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A  , 10, 2   * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A  , 10, 2   * TICKS_PER_SECOND);
        env.console.botbase().wait_for_all_requests();

        env.log("Check the stats.", "purple");
        for (int i = 0; i < 10; i++){
            pbf_press_button(env.console, BUTTON_B, 10, 1 * TICKS_PER_SECOND);
        }
        pbf_press_button(env.console, BUTTON_X , 10, 2   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_LEFT, 10, 0.5 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A , 10, 2   * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_R , 10, 3   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_LEFT, 10, 1   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_UP  , 10, 1   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_UP  , 10, 1   * TICKS_PER_SECOND);

        if (CHECK_HORSE_STATS){
            env.console.botbase().wait_for_all_requests();
            IVCheckerReaderScope reader(m_iv_checker_reader, env.console, LANGUAGE);
            IVCheckerReader::Results results = reader.read(env.console, env.console.video().snapshot());
            bool horse_ok = true;
            horse_ok &= HORSE_HP.matches(stats.errors, results.hp);
            horse_ok &= HORSE_ATTACK.matches(stats.errors, results.attack);
            horse_ok &= HORSE_DEFENSE.matches(stats.errors, results.defense);
            horse_ok &= HORSE_SPATK.matches(stats.errors, results.spatk);
            horse_ok &= HORSE_SPDEF.matches(stats.errors, results.spdef);
            horse_ok &= HORSE_SPEED.matches(stats.errors, results.speed);
            if (horse_ok){
                match_found = true;
            }
        }
        if (CHECK_CALYREX_STATS){
            pbf_press_dpad(env.console, DPAD_UP, 10, 1 * TICKS_PER_SECOND);
            env.console.botbase().wait_for_all_requests();

            IVCheckerReaderScope reader(m_iv_checker_reader, env.console, LANGUAGE);
            IVCheckerReader::Results results = reader.read(env.console, env.console.video().snapshot());
            bool calyrex_ok = true;
            calyrex_ok &= CALYREX_HP.matches(stats.errors, results.hp);
            calyrex_ok &= CALYREX_ATTACK.matches(stats.errors, results.attack);
            calyrex_ok &= CALYREX_DEFENSE.matches(stats.errors, results.defense);
            calyrex_ok &= CALYREX_SPATK.matches(stats.errors, results.spatk);
            calyrex_ok &= CALYREX_SPDEF.matches(stats.errors, results.spdef);
            calyrex_ok &= CALYREX_SPEED.matches(stats.errors, results.speed);
            if (calyrex_ok){
                match_found = true;
            }
        }
        if (!match_found){
            pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
            reset_game_from_home_with_inference(
                env, env.console,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
        }
    }
    stats.matches++;
    env.log("Result Found!", Qt::blue);
    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "Found a perfect match!",
        stats.to_str()
    );
    GO_HOME_WHEN_DONE.run_end_of_program(env.console);
}


}
}
}
