/*  Stats Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/FixedInterval.h"
//#include "PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_StatsReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StatsReset_Descriptor::StatsReset_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:StatsReset",
        "Stats Reset",
        "SwSh-Arduino/wiki/Advanced:-StatsReset",
        "Repeatedly receive gift " + STRING_POKEMON + " until you get the stats you want.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



StatsReset::StatsReset(const StatsReset_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b>",
        m_iv_checker_reader.languages()
    )
    , POKEMON(
        "<b>Gift " + STRING_POKEMON + ":</b>",
        {
            "Type: Null",
            "Cosmog",
            "Poipole",
        },
        0
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
    PA_ADD_OPTION(POKEMON);
    PA_ADD_OPTION(HP);
    PA_ADD_OPTION(ATTACK);
    PA_ADD_OPTION(DEFENSE);
    PA_ADD_OPTION(SPATK);
    PA_ADD_OPTION(SPDEF);
    PA_ADD_OPTION(SPEED);
}



struct StatsReset::Stats : public StatsTracker{
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back(Stat("Attempts"));
        m_display_order.emplace_back(Stat("Errors"));
        m_display_order.emplace_back(Stat("Matches"));
    }

    uint64_t& attempts;
    uint64_t& errors;
    uint64_t& matches;
};
std::unique_ptr<StatsTracker> StatsReset::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



void StatsReset::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    Stats& stats = env.stats<Stats>();

    while (true){
        env.update_stats();

        env.console.botbase().wait_for_all_requests();
        {
            BlackScreenDetector detector;
            int result = run_until(
                env, env.console,
                [=](const BotBaseContext& context){
                    if (POKEMON == 2){
                        pbf_mash_button(context, BUTTON_A, 10 * TICKS_PER_SECOND);
                    }else{
                        pbf_mash_button(context, BUTTON_A, 5 * TICKS_PER_SECOND);
                    }
                    pbf_mash_button(context, BUTTON_B, 20 * TICKS_PER_SECOND);
                    context->wait_for_all_requests();
                },
                { &detector }
            );
            if (result == 0){
                env.log(STRING_POKEMON + " receive menu detected.", "purple");
            }else{
                env.log(STRING_POKEMON + " receive menu timed out.", Qt::red);
            }
        }
        stats.attempts++;

        pbf_mash_button(env.console, BUTTON_B, 1 * TICKS_PER_SECOND);

        pbf_press_button(env.console, BUTTON_X, 10, OVERWORLD_TO_MENU_DELAY);
        ssf_press_dpad2(env.console, DPAD_RIGHT, BOX_SCROLL_DELAY, 10);
        ssf_press_button2(env.console, BUTTON_A, MENU_TO_POKEMON_DELAY, 10);
        ssf_press_button2(env.console, BUTTON_R, POKEMON_TO_BOX_DELAY, 10);
        env.console.botbase().wait_for_all_requests();

        {
            IVCheckerReaderScope reader(m_iv_checker_reader, env.console, LANGUAGE);
            IVCheckerReader::Results results = reader.read(env.console, env.console.video().snapshot());
            bool ok = true;
            ok &= HP.matches(stats.errors, results.hp);
            ok &= ATTACK.matches(stats.errors, results.attack);
            ok &= DEFENSE.matches(stats.errors, results.defense);
            ok &= SPATK.matches(stats.errors, results.spatk);
            ok &= SPDEF.matches(stats.errors, results.spdef);
            ok &= SPEED.matches(stats.errors, results.speed);
            if (ok){
                break;
            }
        }

        //  Add a little extra wait time since correctness matters here.
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);

        reset_game_from_home_with_inference(
            env, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW
        );
    }

    stats.matches++;
    env.update_stats();
    env.log("Result Found!", Qt::blue);

    pbf_wait(env.console, 5 * TICKS_PER_SECOND);
    pbf_press_button(env.console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}


}
}
}
