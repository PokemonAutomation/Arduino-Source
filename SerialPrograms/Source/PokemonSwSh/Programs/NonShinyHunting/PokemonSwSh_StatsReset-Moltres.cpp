/*  Stats Reset - Moltres
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_StatsReset-Moltres.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StatsResetMoltres_Descriptor::StatsResetMoltres_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:StatsResetMoltres",
        STRING_POKEMON + " SwSh", "Stats Reset - Moltres",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/StatsReset-Moltres.md",
        "Repeatedly catch moltres until you get the stats you want.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct StatsResetMoltres_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , errors(m_stats["Errors"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back(Stat("Resets"));
        m_display_order.emplace_back(Stat("Errors"));
        m_display_order.emplace_back(Stat("Matches"));
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& matches;
};
std::unique_ptr<StatsTracker> StatsResetMoltres_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




StatsResetMoltres::StatsResetMoltres()
    : GO_HOME_WHEN_DONE(false)
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
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(HP);
    PA_ADD_OPTION(ATTACK);
    PA_ADD_OPTION(DEFENSE);
    PA_ADD_OPTION(SPATK);
    PA_ADD_OPTION(SPDEF);
    PA_ADD_OPTION(SPEED);

    PA_ADD_OPTION(NOTIFICATIONS);
}



void StatsResetMoltres::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    StatsResetMoltres_Descriptor::Stats& stats = env.current_stats<StatsResetMoltres_Descriptor::Stats>();

    while (true){
        context.wait_for_all_requests();
        env.log("Wait for moltres to attack you.", COLOR_PURPLE);
        {
            StandardBattleMenuWatcher fight_detector(false);
            int result = run_until(
                env.console, context,
                [](BotBaseContext& context){
                    while (true){
                        pbf_wait(context, 1 * TICKS_PER_SECOND);
                    }
                },
                {{fight_detector}}
            );
            if (result == 0){
                env.log("New fight detected.", COLOR_PURPLE);
                pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);
            }
        }

        context.wait_for_all_requests();
        CatchResults result = basic_catcher(env.console, context, LANGUAGE, "master-ball", 999);
        if (result.result != CatchResult::POKEMON_CAUGHT){
            OperationFailedException::fire(
                env.console, ErrorReport::SEND_ERROR_REPORT,
                "Unable to catch Moltres."
            );
        }

        context.wait_for_all_requests();
        env.log("Exit the fight.", COLOR_PURPLE);
        for (int i = 0; i < 20; i++){
            pbf_press_button(context, BUTTON_B, 10, 1 * TICKS_PER_SECOND);
        }

        context.wait_for_all_requests();
        env.log("Check the stats.", COLOR_PURPLE);
        pbf_press_button(context, BUTTON_X , 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
        pbf_press_dpad  (context, DPAD_UP  , 10, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_A , 10, 2   * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_R , 10, 3   * TICKS_PER_SECOND);
        pbf_press_dpad  (context, DPAD_LEFT, 10, 1   * TICKS_PER_SECOND);
        pbf_press_dpad  (context, DPAD_UP  , 10, 1   * TICKS_PER_SECOND);
        pbf_press_dpad  (context, DPAD_UP  , 10, 1   * TICKS_PER_SECOND);

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
        if (ok){
            break;
        }else{
            stats.resets++;
            env.update_stats();
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
            reset_game_from_home_with_inference(
                env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );

            context.wait_for_all_requests();
            env.log("Wait for moltres to attack you.", COLOR_PURPLE);
            {
                StandardBattleMenuWatcher fight_detector(false);
                int ret = run_until(
                    env.console, context,
                    [](BotBaseContext& context){
                        while (true){
                            pbf_wait(context, 1 * TICKS_PER_SECOND);
                        }
                    },
                    {{fight_detector}}
                );
                if (ret == 0){
                    env.log("New fight detected.", COLOR_PURPLE);
                    pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);
                    pbf_press_dpad(context, DPAD_UP , 10, 1 * TICKS_PER_SECOND);
                    pbf_press_button(context, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
                }
            }
            for (int i = 0; i < 10; i++){
                pbf_press_button(context, BUTTON_B, 10, 1 * TICKS_PER_SECOND);
            }

            context.wait_for_all_requests();
            env.log("Let's camp.", COLOR_PURPLE);
            pbf_press_button(context, BUTTON_X  , 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
            pbf_press_dpad  (context, DPAD_RIGHT, 10, 1 * TICKS_PER_SECOND);
            pbf_press_dpad  (context, DPAD_DOWN , 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A  , 10, 8 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_X  , 10, 1 * TICKS_PER_SECOND);
            pbf_press_dpad  (context, DPAD_LEFT , 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A  , 10, 7 * TICKS_PER_SECOND);

            context.wait_for_all_requests();
            env.log("Let's save.", COLOR_PURPLE);
            pbf_press_button(context, BUTTON_X , 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
            pbf_press_button(context, BUTTON_R , 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A , 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A , 10, 1 * TICKS_PER_SECOND);
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
