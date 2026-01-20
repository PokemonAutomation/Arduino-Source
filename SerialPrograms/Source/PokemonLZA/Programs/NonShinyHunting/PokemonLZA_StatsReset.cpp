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
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/Battles/PokemonLZA_RunFromBattleDetector.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_IvJudgeReader.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA_StatsReset.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
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
        , catch_fail(m_stats["Catch Fails"])
        , errors(m_stats["Errors"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back(Stat("Attempts"));
        m_display_order.emplace_back(Stat("Catch Fails"));
        m_display_order.emplace_back(Stat("Errors"));
        m_display_order.emplace_back(Stat("Matches"));
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& catch_fail;
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
            {GiftPokemon::FLOETTE,  "floette",  "Floette" },
            {GiftPokemon::MAGEARNA, "magearna", "Magearna"},
            {GiftPokemon::MELTAN,   "meltan",   "Meltan"  },
            {GiftPokemon::MELMETAL, "melmetal", "Melmetal"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        GiftPokemon::FLOETTE
    )
    , RIGHT_SCROLLS(
        "<b>" + STRING_POKEBALL + " Right-Scrolls:</b><br>"
                                  "Scroll this many balls to the right. Negative will scroll to the left.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0, -15, 15
    )
    , SCROLL_HOLD(
        "<b>Scroll Hold:</b><br>"
        "When scrolling to the desired ball, hold the dpad button for this long.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "120 ms"
    )
    , SCROLL_RELEASE(
        "<b>Scroll Release:</b><br>"
        "When scrolling to the desired ball, release dpad button for this long before the next scroll.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "80 ms"
    )
    , POST_THROW_WAIT(
        "<b>Post-Throw Wait:</b><br>Wait this long after throwing a ball before resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "6000 ms"
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
    PA_ADD_OPTION(RIGHT_SCROLLS);
    PA_ADD_OPTION(SCROLL_HOLD);
    PA_ADD_OPTION(SCROLL_RELEASE);
    PA_ADD_OPTION(POST_THROW_WAIT);
    PA_ADD_OPTION(HP);
    PA_ADD_OPTION(ATTACK);
    PA_ADD_OPTION(DEFENSE);
    PA_ADD_OPTION(SPATK);
    PA_ADD_OPTION(SPDEF);
    PA_ADD_OPTION(SPEED);

    PA_ADD_OPTION(NOTIFICATIONS);

    StatsReset::on_config_value_changed(this);
    POKEMON.add_listener(*this);
}

StatsReset::~StatsReset(){
    POKEMON.remove_listener(*this);
}

void StatsReset::on_config_value_changed(void* object){
    ConfigOptionState state = POKEMON == GiftPokemon::MELMETAL
                                  ? ConfigOptionState::ENABLED
                                  : ConfigOptionState::HIDDEN;
    RIGHT_SCROLLS.set_visibility(state);
    SCROLL_HOLD.set_visibility(state);
    SCROLL_RELEASE.set_visibility(state);
    POST_THROW_WAIT.set_visibility(state);
}

void StatsReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);


    StatsReset_Descriptor::Stats& stats = env.current_stats<StatsReset_Descriptor::Stats>();

    VideoSnapshot screen;
    while (true){
        env.update_stats();
        context.wait_for_all_requests();

        if (POKEMON == GiftPokemon::FLOETTE || POKEMON == GiftPokemon::MAGEARNA){
            // fly to Quasartico, replace with OCR in the future
            overworld_to_main_menu(env.console, context);
            open_map(env.console, context, false, false);
            pbf_move_right_joystick(context, {0, +1}, 500ms, 500ms);
            pbf_move_right_joystick(context, {0, +1}, 500ms, 500ms);
            pbf_move_left_joystick(context, {-0.609, 0}, 100ms, 500ms);
            fly_from_map(env.console, context);

            // move to the door
            pbf_move_left_joystick(context, {0, +1}, 8s, 500ms);
            if (POKEMON == GiftPokemon::FLOETTE){
                pbf_mash_button(context, BUTTON_A, 30s);
            }
            if (POKEMON == GiftPokemon::MAGEARNA){
                pbf_mash_button(context, BUTTON_A, 4s);
                pbf_move_left_joystick(context, {+0.1, +1}, 2s, 500ms);
                pbf_mash_button(context, BUTTON_A, 30s);
            }
        }

        if (POKEMON == GiftPokemon::MELTAN || POKEMON == GiftPokemon::MELMETAL){
            // fly to Lysandre Café, replace with OCR in the future
            overworld_to_main_menu(env.console, context);
            open_map(env.console, context, false, false);
            pbf_press_button(context, BUTTON_Y, 50ms, 500ms);
            pbf_press_dpad(context, DPAD_RIGHT, 50ms, 500ms);
            pbf_press_dpad(context, DPAD_RIGHT, 50ms, 500ms);
            pbf_press_dpad(context, DPAD_RIGHT, 50ms, 500ms);
            pbf_press_dpad(context, DPAD_UP, 50ms, 500ms);
            pbf_mash_button(context, BUTTON_A, 5s);

            // move to the staircase
            pbf_move_left_joystick(context, {-1, -0.05}, 200ms, 500ms);
            pbf_press_button(context, BUTTON_L, 50ms, 500ms);
            ssf_press_button(context, BUTTON_B, 0ms, 1000ms, 0ms);
            pbf_move_left_joystick(context, {0, +1}, 6s, 500ms);
            pbf_move_left_joystick(context, {1, 0.45}, 200ms, 500ms);
            pbf_press_button(context, BUTTON_L, 50ms, 500ms);
            ssf_press_button(context, BUTTON_B, 0ms, 1000ms, 0ms);
            pbf_move_left_joystick(context, {0, +1}, 8s, 500ms);
            if (POKEMON == GiftPokemon::MELTAN){
                // start battle, knock out in one move
                RunFromBattleWatcher battle_menu(COLOR_GREEN, &env.console.overlay(), 10ms);

                context.wait_for_all_requests();
                int ret = run_until<ProControllerContext>(
                    env.console, context,
                    [](ProControllerContext& context){
                        pbf_mash_button(context, BUTTON_B, 120s);
                    },
                    {
                        battle_menu,
                    }
                    );

                if (ret == 0){
                    env.log("Detected battle menu");
                    pbf_press_button(context, BUTTON_Y, 50ms, 500ms);
                    ssf_press_button(context, BUTTON_ZL, 0ms, 4s, 200ms);
                    pbf_mash_button(context, BUTTON_A, 4s);
                    context.wait_for_all_requests();
                }

                // expected knock out, throw ball
                int8_t scrolls = RIGHT_SCROLLS;
                DpadPosition direction;
                if (scrolls >= 0){
                    direction = DPAD_RIGHT;
                }else{
                    direction = DPAD_LEFT;
                    scrolls = -scrolls;
                }

                Milliseconds hold = SCROLL_HOLD;
                Milliseconds cool = SCROLL_RELEASE;

                ssf_press_button(
                    context,
                    BUTTON_ZL | BUTTON_ZR,
                    500ms, 500ms + (hold + cool) * scrolls,
                    0ms
                    );

                while (scrolls != 0){
                    pbf_press_dpad(context, direction, hold, cool);
                    scrolls--;
                }

                pbf_wait(context, POST_THROW_WAIT);
                pbf_mash_button(context, BUTTON_A, 10s);
            }
            if (POKEMON == GiftPokemon::MELMETAL){
                // enter portal
                pbf_press_button(context, BUTTON_A, 50ms, 1s);
                pbf_press_dpad(context, DPAD_UP, 50ms, 500ms);
                pbf_mash_button(context, BUTTON_A, 5s);
                pbf_press_button(context, BUTTON_PLUS, 5s, 500ms);

                OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());

                context.wait_for_all_requests();
                int ret = run_until<ProControllerContext>(
                    env.console, context,
                    [](ProControllerContext& context){
                        pbf_mash_button(context, BUTTON_B, 30s);
                    },
                    {
                        overworld,
                    }
                    );

                if (ret == 0){
                    env.log("Detected overworld");
                    context.wait_for_all_requests();
                }

                pbf_press_button(context, BUTTON_L, 50ms, 500ms);
                ssf_press_button(context, BUTTON_B, 0ms, 1000ms, 0ms);
                pbf_move_left_joystick(context, {0, +1}, 2s, 1s);
                pbf_move_left_joystick(context, {+1, 0}, 200ms, 500ms);
                pbf_press_button(context, BUTTON_L, 50ms, 500ms);
                ssf_press_button(context, BUTTON_B, 0ms, 1000ms, 0ms);
                pbf_move_left_joystick(context, {0, +1}, 1s, 1s);
                pbf_move_left_joystick(context, {-1, +1}, 200ms, 500ms);
                pbf_press_button(context, BUTTON_L, 50ms, 500ms);
                ssf_press_button(context, BUTTON_B, 0ms, 1000ms, 0ms);
                pbf_move_left_joystick(context, {+0.05, +1}, 3s, 1s);

                pbf_mash_button(context, BUTTON_A, 4s);
            }
        }

        context.wait_for_all_requests();
        {
            BlackScreenOverWatcher detector;
            OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
            int result = run_until<ProControllerContext>(
                env.console, context,
                [this](ProControllerContext& context){
                    if (POKEMON == GiftPokemon::FLOETTE || POKEMON == GiftPokemon::MELMETAL){
                        pbf_mash_button(context, BUTTON_A, 60s);
                    }else if (POKEMON == GiftPokemon::MAGEARNA){
                        pbf_mash_button(context, BUTTON_A, 50s);
                    }else{
                        pbf_mash_button(context, BUTTON_A, 30s);
                    }
                },
                {
                    detector,
                    overworld,
                }
                );
            switch (result){
            case 0:
                env.log(STRING_POKEMON + " dialog finished.", COLOR_PURPLE);
                break;
            case 1:
                env.log("Early overworld, catch failed", COLOR_PURPLE);
                stats.catch_fail++;
                go_home(env.console, context);
                reset_game_from_home(env, env.console, context, true);
                continue;
            default:
                env.log(STRING_POKEMON + " dialog timed out.", COLOR_RED);
                // fail safely and start over
                go_home(env.console, context);
                reset_game_from_home(env, env.console, context, true);
                continue;
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

