/*  Stats Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_IvJudgeReader.h"
#include "PokemonLZA_StatsReset.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA/Programs/PokemonLZA_MenuNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


StatsReset_Descriptor::StatsReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:StatsReset",
        STRING_POKEMON + " LZA", "Stats Reset",
        "Programs/PokemonLZA/StatsReset.html",
        "Repeatedly receive gift " + STRING_POKEMON + " until you get the stats you want.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct StatsReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back(Stat("Attempts"));
        m_display_order.emplace_back(Stat("Errors"));
        m_display_order.emplace_back(Stat("Matches"));
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& matches;
};
std::unique_ptr<StatsTracker> StatsReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



StatsReset::StatsReset()
    : GO_HOME_WHEN_DONE(true)
    , LANGUAGE(
        "<b>Game Language:</b>",
        IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING
    )
    , POKEMON(
        "<b>Gift " + STRING_POKEMON + ":</b>",
        {
            {GiftPokemon::FLOETTE, "floette", "Floette"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        GiftPokemon::FLOETTE
    )
    , HP("<b>HP:</b>")
    , ATTACK("<b>Attack:</b>", IvJudgeFilter::NoGood)
    , DEFENSE("<b>Defense:</b>")
    , SPATK("<b>Sp. Atk:</b>")
    , SPDEF("<b>Sp. Def:</b>")
    , SPEED("<b>Speed:</b>")
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true, ImageAttachmentMode::JPG)
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(POKEMON);
    PA_ADD_OPTION(HP);
    PA_ADD_OPTION(ATTACK);
    PA_ADD_OPTION(DEFENSE);
    PA_ADD_OPTION(SPATK);
    PA_ADD_OPTION(SPDEF);
    PA_ADD_OPTION(SPEED);

    PA_ADD_OPTION(NOTIFICATIONS);
}



void StatsReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);


    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();

    VideoSnapshot screen;
    while (true){
        env.update_stats();
        context.wait_for_all_requests();

        overworld_to_main_menu(env.console, context);
        pbf_press_button(context, BUTTON_PLUS, 500ms, 500ms);
        pbf_move_right_joystick(context, {0, +1}, 500ms, 500ms);
        pbf_move_right_joystick(context, {0, +1}, 500ms, 500ms);
        pbf_move_left_joystick(context, 50, 128, 100ms, 500ms);
        pbf_mash_button(context, BUTTON_A, 5000ms);

        pbf_move_left_joystick(context, {0, +1}, 10s, 500ms);
        pbf_mash_button(context, BUTTON_A, 30s);

        context.wait_for_all_requests();
        {
            BlackScreenOverWatcher detector;
            int result = run_until<ProControllerContext>(
                env.console, context,
                [this](ProControllerContext& context){
                    if (POKEMON == GiftPokemon::FLOETTE){
                        pbf_mash_button(context, BUTTON_A, 60s);
                    }else{
                        pbf_mash_button(context, BUTTON_A, 30s);
                    }
                },
                {{detector}}
                );
            if (result == 0){
                env.log(STRING_POKEMON + " dialog finished.", COLOR_PURPLE);
            }else{
                env.log(STRING_POKEMON + " dialog timed out.", COLOR_RED);
            }
        }

        overworld_to_box_system(env.console, context);
        pbf_press_button(context, BUTTON_RCLICK, 500ms, 500ms);
        stats.attempts++;
        context.wait_for_all_requests();

        {
            IvJudgeReaderScope reader(env.console, LANGUAGE);
            screen = env.console.video().snapshot();
            IvJudgeReader::Results results = reader.read(env.console, screen);
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

        pbf_mash_button(context, BUTTON_B, 3s);
        go_home(env.console, context);
        reset_game_from_home(env, env.console, context, true);
    }

    stats.matches++;
    env.update_stats();
    env.log("Result Found!", COLOR_BLUE);

    pbf_wait(context, 5000ms);
    pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 5000ms);

    send_program_finished_notification(
        env, NOTIFICATION_PROGRAM_FINISH,
        "Found a match!",
        screen, false
    );
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


}
}
}

