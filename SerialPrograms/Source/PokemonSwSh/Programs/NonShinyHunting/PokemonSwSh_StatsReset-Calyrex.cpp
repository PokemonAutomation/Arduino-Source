/*  Stats Reset - Calyrex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_StatsReset-Calyrex.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StatsResetCalyrex_Descriptor::StatsResetCalyrex_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:StatsResetCalyrex",
        STRING_POKEMON + " SwSh", "Stats Reset - Calyrex",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/StatsReset-Calyrex.md",
        "Repeatedly catch calyrex (and its horse) until you get the stats you want.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct StatsResetCalyrex_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> StatsResetCalyrex_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



StatsResetCalyrex::StatsResetCalyrex()
    : GO_HOME_WHEN_DONE(false)
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "master-ball"
    )
    , LANGUAGE(
        "<b>Game Language:</b>",
        IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , CHECK_CALYREX_STATS(
        "<b>Check Calyrex stats</b>",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , CALYREX_HP("<b>Calyrex HP:</b>")
    , CALYREX_ATTACK("<b>Calyrex Attack:</b>", IvJudgeFilter::NoGood)
    , CALYREX_DEFENSE("<b>Calyrex Defense:</b>")
    , CALYREX_SPATK("<b>Calyrex Sp. Atk:</b>")
    , CALYREX_SPDEF("<b>Calyrex Sp. Def:</b>")
    , CALYREX_SPEED("<b>Calyrex Speed:</b>")
    , CHECK_HORSE_STATS(
        "<b>Check Horse stats</b>",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , HORSE_HP("<b>Horse HP:</b>")
    , HORSE_ATTACK("<b>Horse Attack:</b>", IvJudgeFilter::NoGood)
    , HORSE_DEFENSE("<b>Horse Defense:</b>")
    , HORSE_SPATK("<b>Horse Sp. Atk:</b>")
    , HORSE_SPDEF("<b>Horse Sp. Def:</b>")
    , HORSE_SPEED("<b>Horse Speed:</b>")
    , NOTIFICATION_CATCH_SUCCESS("Catch Success", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_CATCH_FAILED("Catch Failed", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_CATCH_SUCCESS,
        &NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{

    PA_ADD_OPTION(START_LOCATION);
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



void StatsResetCalyrex::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    StatsResetCalyrex_Descriptor::Stats& stats = env.current_stats<StatsResetCalyrex_Descriptor::Stats>();

    bool match_found = false;
    while (!match_found){

        bool calyrex_caught = false;
        while (!calyrex_caught){
            env.log("Talk to calyrex.", COLOR_PURPLE);
            context.wait_for_all_requests();
            {
                StandardBattleMenuWatcher fight_detector(false);
                int result = run_until<ProControllerContext>(
                    env.console, context,
                    [](ProControllerContext& context){
                        while (true){
                            pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                        }
                    },
                    {{fight_detector}}
                );
                if (result == 0){
                    env.log("New fight detected, let's begin to throw balls.", COLOR_PURPLE);
                    pbf_mash_button(context, BUTTON_B, 1000ms);
                }
            }

            pbf_mash_button(context, BUTTON_B, 1000ms);
            CatchResults result = basic_catcher(env.console, context, LANGUAGE, BALL_SELECT.slug(), 999);
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
            case CatchResult::BALL_LIMIT_REACHED:
            case CatchResult::CANNOT_THROW_BALL:
            case CatchResult::TIMED_OUT:
                stats.errors++;
                break;
            }
            stats.total_balls_thrown += result.balls_used;
            env.update_stats();

            if (calyrex_caught){
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

            if (!calyrex_caught){
                ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 160ms);
                reset_game_from_home_with_inference(
                    env.console, context,
                    ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
                );
            }
        }

        env.log("Unfuse calyrex.", COLOR_PURPLE);
        for (int i = 0; i < 40; i++){
            pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
        }
        pbf_press_button(context, BUTTON_X  , 80ms, 1500ms);
        pbf_press_dpad  (context, DPAD_RIGHT, 80ms, 500ms);
        pbf_press_dpad  (context, DPAD_RIGHT, 80ms, 500ms);
        pbf_press_button(context, BUTTON_A  , 80ms, 2000ms);
        pbf_press_dpad  (context, DPAD_LEFT , 80ms, 500ms);
        pbf_press_dpad  (context, DPAD_UP   , 80ms, 500ms);
        pbf_press_button(context, BUTTON_A  , 80ms, 2000ms);
        pbf_press_button(context, BUTTON_A  , 80ms, 2000ms);
        pbf_press_dpad  (context, DPAD_UP   , 80ms, 2000ms);
        pbf_press_button(context, BUTTON_A  , 80ms, 2000ms);
        pbf_press_button(context, BUTTON_A  , 80ms, 2000ms);
        context.wait_for_all_requests();

        env.log("Check the stats.", COLOR_PURPLE);
        for (int i = 0; i < 10; i++){
            pbf_press_button(context, BUTTON_B, 80ms, 1000ms);
        }
        pbf_press_button(context, BUTTON_X , 80ms, 2000ms);
        pbf_press_dpad  (context, DPAD_LEFT, 80ms, 500ms);
        pbf_press_button(context, BUTTON_A , 80ms, 2000ms);
        pbf_press_button(context, BUTTON_R , 80ms, 3000ms);
        pbf_press_dpad  (context, DPAD_LEFT, 80ms, 1000ms);
        pbf_press_dpad  (context, DPAD_UP  , 80ms, 1000ms);
        pbf_press_dpad  (context, DPAD_UP  , 80ms, 1000ms);

        if (CHECK_HORSE_STATS){
            context.wait_for_all_requests();
            IvJudgeReaderScope reader(env.console, LANGUAGE);
            IvJudgeReader::Results results = reader.read(env.console, env.console.video().snapshot());
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
            pbf_press_dpad(context, DPAD_UP, 80ms, 1000ms);
            context.wait_for_all_requests();
            
            IvJudgeReaderScope reader(env.console, LANGUAGE);
            IvJudgeReader::Results results = reader.read(env.console, env.console.video().snapshot());
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
            ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 160ms);
            reset_game_from_home_with_inference(
                env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
        }
    }
    stats.matches++;
    env.log("Result Found!", COLOR_BLUE);
    env.update_stats();
    send_program_finished_notification(
        env, NOTIFICATION_PROGRAM_FINISH,
        "Found a perfect match!"
    );
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


}
}
}
