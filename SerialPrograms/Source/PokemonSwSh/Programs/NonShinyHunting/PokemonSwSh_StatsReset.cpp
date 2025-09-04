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
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BoxHelpers.h"
#include "PokemonSwSh_StatsReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StatsReset_Descriptor::StatsReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:StatsReset",
        STRING_POKEMON + " SwSh", "Stats Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/StatsReset.md",
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
    : GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b>",
        IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING
    )
    , POKEMON(
        "<b>Gift " + STRING_POKEMON + ":</b>",
        {
            {GiftPokemon::TypeNull, "type-null",    "Type: Null"},
            {GiftPokemon::Cosmog,   "cosmog",       "Cosmog"},
            {GiftPokemon::Poipole,  "poipole",      "Poipole"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        GiftPokemon::TypeNull
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
    PA_ADD_OPTION(START_LOCATION);
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
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();

    VideoSnapshot screen;
    while (true){
        env.update_stats();

        context.wait_for_all_requests();
        {
            BlackScreenOverWatcher detector;
            int result = run_until<ProControllerContext>(
                env.console, context,
                [this](ProControllerContext& context){
                    if (POKEMON == GiftPokemon::TypeNull){
                        pbf_mash_button(context, BUTTON_A, 10s);
                    }else{
                        pbf_mash_button(context, BUTTON_A, 5s);
                    }
                    pbf_mash_button(context, BUTTON_B, 20s);
                },
                {{detector}}
            );
            if (result == 0){
                env.log(STRING_POKEMON + " receive menu detected.", COLOR_PURPLE);
            }else{
                env.log(STRING_POKEMON + " receive menu timed out.", COLOR_RED);
            }
        }
        stats.attempts++;

        pbf_mash_button(context, BUTTON_B, 1000ms);

        ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0, 160ms);
        box_scroll(context, DPAD_RIGHT);
        ssf_press_button(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY0, 160ms);
        ssf_press_button(context, BUTTON_R, GameSettings::instance().POKEMON_TO_BOX_DELAY0, 160ms);
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

        //  Add a little extra wait time since correctness matters here.
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 160ms);

        reset_game_from_home_with_inference(
            env.console, context,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
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
