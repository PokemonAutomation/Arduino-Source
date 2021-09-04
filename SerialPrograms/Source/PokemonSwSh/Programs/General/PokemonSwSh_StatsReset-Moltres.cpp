/*  Stats Reset - Moltres
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "PokemonSwSh_StatsReset-Moltres.h"
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


StatsResetMoltres_Descriptor::StatsResetMoltres_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:StatsResetMoltres",
        "Stats Reset - Moltres",
        "SwSh-Arduino/wiki/Advanced:-StatsResetMoltres",
        "Repeatedly catch moltres until you get the stats you want.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



StatsResetMoltres::StatsResetMoltres(const StatsResetMoltres_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
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

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(HP);
    PA_ADD_OPTION(ATTACK);
    PA_ADD_OPTION(DEFENSE);
    PA_ADD_OPTION(SPATK);
    PA_ADD_OPTION(SPDEF);
    PA_ADD_OPTION(SPEED);
}



struct StatsResetMoltres::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , errors(m_stats["Errors"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back(Stat("Resets"));
        m_display_order.emplace_back(Stat("Errors"));
        m_display_order.emplace_back(Stat("Matches"));
    }

    uint64_t& resets;
    uint64_t& errors;
    uint64_t& matches;
};
std::unique_ptr<StatsTracker> StatsResetMoltres::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



void StatsResetMoltres::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    Stats& stats = env.stats<Stats>();

    while (true){
        env.console.botbase().wait_for_all_requests();
        env.log("Wait for moltres to attack you.", "purple");
        {
            InterruptableCommandSession commands(env.console);

            StandardBattleMenuDetector fight_detector(env.console, false);
            fight_detector.register_command_stop(commands);

            AsyncVisualInferenceSession inference(env, env.console);
            inference += fight_detector;

            commands.run([=](const BotBaseContext& context){
                while (true){
                    pbf_wait(context, 1 * TICKS_PER_SECOND);
                }
                context->wait_for_all_requests();
            });

            if (fight_detector.triggered()){
                env.log("New fight detected.", "purple");
                pbf_mash_button(env.console, BUTTON_B, 1 * TICKS_PER_SECOND);
            }
        }

        env.console.botbase().wait_for_all_requests();
        CatchResults result = basic_catcher(env, env.console, LANGUAGE, "master-ball");
        if (result.result != CatchResult::POKEMON_CAUGHT){
            PA_THROW_StringException("Unable to catch Moltres.");
        }

        env.console.botbase().wait_for_all_requests();
        env.log("Exit the fight.", "purple");
        for (int i = 0; i < 20; i++){
            pbf_press_button(env.console, BUTTON_B, 10, 1 * TICKS_PER_SECOND);
        }

        env.console.botbase().wait_for_all_requests();
        env.log("Check the stats.", "purple");
        pbf_press_button(env.console, BUTTON_X , 10, OVERWORLD_TO_MENU_DELAY);
        pbf_press_dpad  (env.console, DPAD_UP  , 10, 0.5 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A , 10, 2   * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_R , 10, 3   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_LEFT, 10, 1   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_UP  , 10, 1   * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_UP  , 10, 1   * TICKS_PER_SECOND);

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
        if (ok){
            break;
        }else{
            stats.resets++;
            env.update_stats();
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            reset_game_from_home_with_inference(
                env, env.console,
                TOLERATE_SYSTEM_UPDATE_MENU_SLOW
            );

            env.console.botbase().wait_for_all_requests();
            env.log("Wait for moltres to attack you.", "purple");
            {
                InterruptableCommandSession commands(env.console);

                StandardBattleMenuDetector fight_detector(env.console, false);
                fight_detector.register_command_stop(commands);

                AsyncVisualInferenceSession inference(env, env.console);
                inference += fight_detector;

                commands.run([=](const BotBaseContext& context){
                    while (true){
                        pbf_wait(context, 1 * TICKS_PER_SECOND);
                    }
                    context->wait_for_all_requests();
                });

                if (fight_detector.triggered()){
                    env.log("New fight detected.", "purple");
                    pbf_mash_button(env.console, BUTTON_B, 1 * TICKS_PER_SECOND);
                    pbf_press_dpad(env.console  , DPAD_UP , 10, 1 * TICKS_PER_SECOND);
                    pbf_press_button(env.console, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
                }
            }
            for (int i = 0; i < 10; i++){
                pbf_press_button(env.console, BUTTON_B, 10, 1 * TICKS_PER_SECOND);
            }

            env.console.botbase().wait_for_all_requests();
            env.log("Let's camp.", "purple");
            pbf_press_button(env.console, BUTTON_X  , 10, OVERWORLD_TO_MENU_DELAY);
            pbf_press_dpad  (env.console, DPAD_RIGHT, 10, 1 * TICKS_PER_SECOND);
            pbf_press_dpad  (env.console, DPAD_DOWN , 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(env.console, BUTTON_A  , 10, 8 * TICKS_PER_SECOND);
            pbf_press_button(env.console, BUTTON_X  , 10, 1 * TICKS_PER_SECOND);
            pbf_press_dpad  (env.console, DPAD_LEFT , 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(env.console, BUTTON_A  , 10, 7 * TICKS_PER_SECOND);

            env.console.botbase().wait_for_all_requests();
            env.log("Let's save.", "purple");
            pbf_press_button(env.console, BUTTON_X , 10, OVERWORLD_TO_MENU_DELAY);
            pbf_press_button(env.console, BUTTON_R , 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(env.console, BUTTON_A , 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(env.console, BUTTON_A , 10, 1 * TICKS_PER_SECOND);
        }
    }

    DiscordWebHook::send_message_old(true, "Found a perfect match", stats.make_discord_stats());
    stats.matches++;
    env.update_stats();
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
