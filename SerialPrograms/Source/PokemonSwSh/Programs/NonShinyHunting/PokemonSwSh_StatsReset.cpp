/*  Stats Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/FixedInterval.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
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
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
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
        LockWhileRunning::LOCKED
    )
    , POKEMON(
        "<b>Gift " + STRING_POKEMON + ":</b>",
        {
            {GiftPokemon::TypeNull, "type-null",    "Type: Null"},
            {GiftPokemon::Cosmog,   "cosmog",       "Cosmog"},
            {GiftPokemon::Poipole,  "poipole",      "Poipole"},
        },
        LockWhileRunning::LOCKED,
        GiftPokemon::TypeNull
    )
    , HP("<b>HP:</b>")
    , ATTACK("<b>Attack:</b>", IVCheckerFilter::NoGood)
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



void StatsReset::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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
            int result = run_until(
                env.console, context,
                [this](BotBaseContext& context){
                    if (POKEMON == GiftPokemon::TypeNull){
                        pbf_mash_button(context, BUTTON_A, 10 * TICKS_PER_SECOND);
                    }else{
                        pbf_mash_button(context, BUTTON_A, 5 * TICKS_PER_SECOND);
                    }
                    pbf_mash_button(context, BUTTON_B, 20 * TICKS_PER_SECOND);
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

        pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);

        pbf_press_button(context, BUTTON_X, 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
        ssf_press_dpad2(context, DPAD_RIGHT, GameSettings::instance().BOX_SCROLL_DELAY, 10);
        ssf_press_button2(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY, 10);
        ssf_press_button2(context, BUTTON_R, GameSettings::instance().POKEMON_TO_BOX_DELAY, 10);
        context.wait_for_all_requests();

        {
            IVCheckerReaderScope reader(env.console, LANGUAGE);
            screen = env.console.video().snapshot();
            IVCheckerReader::Results results = reader.read(env.console, screen);
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
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);

        reset_game_from_home_with_inference(
            env.console, context,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
    }

    stats.matches++;
    env.update_stats();
    env.log("Result Found!", COLOR_BLUE);

    pbf_wait(context, 5 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);

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
