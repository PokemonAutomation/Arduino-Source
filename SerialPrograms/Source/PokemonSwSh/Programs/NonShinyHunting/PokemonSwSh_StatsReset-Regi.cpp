/*  Stats Reset - Regi
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
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_StatsReset-Regi.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StatsResetRegi_Descriptor::StatsResetRegi_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:StatsResetRegi",
        STRING_POKEMON + " SwSh", "Stats Reset - Regi",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/StatsReset-Regi.md",
        "Repeatedly catch regi until you get the stats you want.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct StatsResetRegi_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> StatsResetRegi_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



StatsResetRegi::StatsResetRegi()
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
    , HP("<b>HP:</b>")
    , ATTACK("<b>Attack:</b>", IvJudgeFilter::NoGood)
    , DEFENSE("<b>Defense:</b>")
    , SPATK("<b>Sp. Atk:</b>")
    , SPDEF("<b>Sp. Def:</b>")
    , SPEED("<b>Speed:</b>")
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
    PA_ADD_OPTION(HP);
    PA_ADD_OPTION(ATTACK);
    PA_ADD_OPTION(DEFENSE);
    PA_ADD_OPTION(SPATK);
    PA_ADD_OPTION(SPDEF);
    PA_ADD_OPTION(SPEED);

    PA_ADD_OPTION(NOTIFICATIONS);
}



void StatsResetRegi::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    StatsResetRegi_Descriptor::Stats& stats = env.current_stats<StatsResetRegi_Descriptor::Stats>();

    bool match_found = false;
    while (!match_found){

        bool regi_caught = false;
        while (!regi_caught){
            env.log("Talk to regi.", COLOR_PURPLE);
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

            env.log("Catch regi.", COLOR_PURPLE);
            CatchResults result = basic_catcher(env.console, context, LANGUAGE, BALL_SELECT.slug(), 999);
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
            case CatchResult::BALL_LIMIT_REACHED:
            case CatchResult::CANNOT_THROW_BALL:
            case CatchResult::TIMED_OUT:
                stats.errors++;
                break;
            }
            stats.total_balls_thrown += result.balls_used;
            env.update_stats();

            if (regi_caught){
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

            if (!regi_caught){
                ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 160ms);
                reset_game_from_home_with_inference(
                    env.console, context,
                    ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
                );
            }
        }

        env.log("Check the stats.", COLOR_PURPLE);
        for (int i = 0; i < 20; i++){
            pbf_press_button(context, BUTTON_B, 80ms, 1000ms);
        }
        pbf_press_button(context, BUTTON_X  , 80ms, 2000ms);
        pbf_press_dpad  (context, DPAD_RIGHT, 80ms, 500ms);
        pbf_press_button(context, BUTTON_A  , 80ms, 2000ms);
        pbf_press_button(context, BUTTON_R  , 80ms, 3000ms);
        pbf_press_dpad  (context, DPAD_LEFT , 80ms, 1000ms);
        pbf_press_dpad  (context, DPAD_UP   , 80ms, 1000ms);
        pbf_press_dpad  (context, DPAD_UP   , 80ms, 1000ms);

        context.wait_for_all_requests();
        IvJudgeReaderScope reader(env.console, LANGUAGE);
        IvJudgeReader::Results results = reader.read(env.console, env.console.video().snapshot());
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
            ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 160ms);
            reset_game_from_home_with_inference(
                env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
        }
    }

    env.log("Result Found!", COLOR_BLUE);
    stats.matches++;

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
