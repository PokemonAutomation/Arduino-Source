/*  Auto Story
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh_AutoStory.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


AutoStory_Descriptor::AutoStory_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:AutoStory",
        STRING_POKEMON + " SwSh", "Auto Story",
        "Programs/PokemonSwSh/AutoStory.html",
        "Run through the base story automatically.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

AutoStory::AutoStory()
    : STARTER(
        "<b>Starter " + STRING_POKEMON + "</b><br>"
        "Select this starter Pokémon.",
        {
            {StarterPokemon::GROOKEY, "grookey", "Grookey"},
            {StarterPokemon::SCORBUNNY, "scorbunny", "Scorbunny"},
            {StarterPokemon::SOBBLE, "sobble", "Sobble"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        StarterPokemon::SOBBLE
    )
    , LANGUAGE(
        "<b>Game Language:</b>",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
            Language::Korean,
            Language::ChineseSimplified,
            Language::ChineseTraditional,
        },
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , BALL_SELECT(
        "<b>Eternatus Ball:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "master-ball"
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STARTER);
    PA_ADD_OPTION(BALL_SELECT);
}

void AutoStory::save_from_overworld(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_press_button(context, BUTTON_R, 160ms, 2s);
    pbf_press_button(context, BUTTON_ZL, 160ms, 5s);
}

void AutoStory::mash_until_black_screen(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Button button){
    BlackScreenOverWatcher black_screen;

    context.wait_for_all_requests();

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [button](ProControllerContext& context){
            pbf_mash_button(context, button, 300s);
        },
        {
            black_screen,
        }
        );

    if (ret == 0){
        env.log("Detected black screen");
        context.wait_for_all_requests();
    }
}

void AutoStory::mash_until_black_screen_multi(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int8_t cycle, Button button){
    for(int i=0; i<cycle; i++){
        mash_until_black_screen(env, context, button);
        pbf_wait(context, 2s);
    }
}

void AutoStory::run_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    mash_until_black_screen(env, context, BUTTON_A);
}

void AutoStory::run_catch(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool use_pokeball){
    std::string ball = "poke-ball";
    if (!use_pokeball){
        ball = BALL_SELECT.slug();
    }

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

    CatchResults result = basic_catcher(env.console, context, LANGUAGE, ball, 999);
    switch (result.result){
    case CatchResult::POKEMON_CAUGHT:
        break;
    case CatchResult::POKEMON_FAINTED:
        break;
    case CatchResult::OWN_FAINTED:
        break;
    case CatchResult::OUT_OF_BALLS:
        break;
    case CatchResult::BALL_LIMIT_REACHED:
    case CatchResult::CANNOT_THROW_BALL:
    case CatchResult::TIMED_OUT:
        break;
    }
}

void AutoStory::overworld_navigation(SingleSwitchProgramEnvironment& env, ProControllerContext& context, double x, double y, uint16_t seconds_timeout, bool is_catch){
    while (true){
        context.wait_for_all_requests();
        BlackScreenOverWatcher black_screen;
        WhiteScreenOverWatcher white_screen;
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                ssf_press_left_joystick(context, {x, y}, 0ms, Seconds(seconds_timeout));
                for (int i = 0; i < 5 * seconds_timeout; i++){
                    pbf_press_button(context, BUTTON_B, 100ms, 100ms);
                }
            },
            {{black_screen, white_screen}}
        );

        switch (ret){
        case 0:
        case 1:
            pbf_wait(context, 2s);
            if (is_catch){
                run_catch(env, context);
            } else {
                run_battle(env, context);
            }
            env.log("Battle");
            break;
        default:
            pbf_wait(context, 2s);
            return;
        }
    }
}

void AutoStory::segment_00(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // mash through the initial cutscene
    pbf_mash_button(context, BUTTON_A, 30s);
    pbf_mash_button(context, BUTTON_B, 180s);

    // update settings
    pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_press_dpad(context, DPAD_RIGHT, 1000ms, 100ms);
    pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_LEFT, 100ms, 100ms);
    for(int i=0; i<3; i++){
        pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
    }
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    for(int i=0; i<3; i++){
        pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
    }
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_mash_button(context, BUTTON_A, 3s);
    pbf_mash_button(context, BUTTON_B, 4s);

    // grab the player's bag
    pbf_move_left_joystick(context, {0, -1}, 700ms, 100ms);
    pbf_move_left_joystick(context, {1, 0}, 4s, 100ms);
    pbf_move_left_joystick(context, {0, 1}, 100ms, 100ms);
    pbf_mash_button(context, BUTTON_A, 15s);

    save_from_overworld(env, context);
}

void AutoStory::segment_01(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // leave home and navigate to Hop's home
    pbf_move_left_joystick(context, {-1, -0.5}, 5s, 100ms);
    pbf_move_left_joystick(context, {0, -1}, 20s, 100ms);
    pbf_mash_button(context, BUTTON_B, 30s);
    pbf_move_left_joystick(context, {1, 0}, 20s, 100ms);
    pbf_move_left_joystick(context, {-1, 0}, 1s, 100ms);
    pbf_move_left_joystick(context, {0, 1}, 2s, 100ms);
    pbf_mash_button(context, BUTTON_B, 30s);

    // Head to the station and talk to Leon
    pbf_move_left_joystick(context, {1, -0.3}, 2s, 100ms);
    pbf_move_left_joystick(context, {0, -1}, 300ms, 2s);
    pbf_move_left_joystick(context, {0, -1}, 300ms, 100ms);
    pbf_move_left_joystick(context, {-1, 0}, 5s, 100ms);
    pbf_move_left_joystick(context, {0, 1}, 5s, 100ms);
    mash_until_black_screen(env, context);
    pbf_move_left_joystick(context, {-0.1, 1}, 30s, 100ms);
    mash_until_black_screen_multi(env, context, 2);
    pbf_mash_button(context, BUTTON_B, 2s);

    // pick a starter and finish the battle tutorial
    switch (STARTER){
    case StarterPokemon::GROOKEY:
        pbf_move_left_joystick(context, {-1, 0}, 850ms, 100ms);
        break;
    case StarterPokemon::SCORBUNNY:
        pbf_move_left_joystick(context, {-1, 0}, 600ms, 100ms);
        break;
    case StarterPokemon::SOBBLE:
        pbf_move_left_joystick(context, {-1, 0}, 350ms, 100ms);
        break;
    }
    pbf_move_left_joystick(context, {0, 1}, 600ms, 100ms);
    mash_until_black_screen_multi(env, context, 9, BUTTON_A);

    save_from_overworld(env, context);
}

void AutoStory::segment_02(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // place the elemental STAB move on the top slot
    pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_press_dpad(context, DPAD_LEFT, 1s, 100ms);
    pbf_press_button(context, BUTTON_A, 160ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 160ms, 500ms);
    pbf_press_button(context, BUTTON_A, 160ms, 2000ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_button(context, BUTTON_A, 160ms, 500ms);
    pbf_press_button(context, BUTTON_A, 160ms, 500ms);
    pbf_press_dpad(context, DPAD_DOWN, 1s, 100ms);
    pbf_press_button(context, BUTTON_A, 160ms, 500ms);
    pbf_mash_button(context, BUTTON_B, 6s);

    // head to the forest
    pbf_move_left_joystick(context, {-0.8, -1}, 2s, 100ms);
    mash_until_black_screen_multi(env, context, 2, BUTTON_A);
    pbf_move_left_joystick(context, {-1, -0.5}, 18s, 100ms);
    pbf_mash_button(context, BUTTON_A, 6s);

    // defeat the 3 mandatory wild battles
    pbf_move_left_joystick(context, {-1, -0.2}, 9s, 100ms);
    mash_until_black_screen(env, context, BUTTON_A);
    pbf_move_left_joystick(context, {-1, -0.2}, 9s, 100ms);
    mash_until_black_screen(env, context, BUTTON_A);
    pbf_move_left_joystick(context, {-1, -0.2}, 9s, 100ms);
    pbf_mash_button(context, BUTTON_A, 3s);
    pbf_move_left_joystick(context, {-1, -0.2}, 9s, 100ms);
    mash_until_black_screen(env, context, BUTTON_A);

    // engage the legendary
    pbf_move_left_joystick(context, {-0.4, 1}, 6s, 100ms);
    pbf_mash_button(context, BUTTON_A, 3s);
    pbf_move_left_joystick(context, {0, 1}, 16s, 100ms);
    mash_until_black_screen_multi(env, context, 2, BUTTON_A);
    pbf_mash_button(context, BUTTON_A, 5s);

    save_from_overworld(env, context);
}

void AutoStory::segment_03(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_move_left_joystick(context, {-0.8, 1}, 5s, 2s);
    pbf_move_left_joystick(context, {1, 0.6}, 3s, 6s);
    mash_until_black_screen(env, context);
    pbf_move_left_joystick(context, {-1, -0.5}, 2s, 100ms);
    pbf_move_left_joystick(context, {0, -1}, 7s, 100ms);

    pbf_move_left_joystick(context, {1, 0.3}, 6s, 100ms);
    pbf_move_left_joystick(context, {-0.2, 1}, 10s, 100ms);
    mash_until_black_screen(env, context);
    pbf_move_left_joystick(context, {0, 1}, 3500ms, 100ms);

    // route 1, attempt to catch along the way
    overworld_navigation(env, context, 1, 0, 15, true);
    overworld_navigation(env, context, 0, 1, 15, true);
    overworld_navigation(env, context, -1, 0, 15, true);

    // enter Laboratory
    pbf_move_left_joystick(context, {0, 1}, 5500ms, 100ms);
    pbf_move_left_joystick(context, {1, 0}, 15s, 100ms);

    mash_until_black_screen_multi(env, context, 2, BUTTON_A);

    save_from_overworld(env, context);
}

void AutoStory::segment_04(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // exit Laboratory, enter Pokemon Center
    pbf_move_left_joystick(context, {0, -1}, 3s, 100ms);
    mash_until_black_screen_multi(env, context, 2);
    pbf_move_left_joystick(context, {-1, -0.3}, 4500ms, 100ms);
    pbf_move_left_joystick(context, {0, 1}, 5000ms, 100ms);
    pbf_mash_button(context, BUTTON_B, 20s);
    pbf_press_dpad(context, DPAD_DOWN, 100ms, 100ms);
    pbf_mash_button(context, BUTTON_A, 12s);

    // getting to route 2
    pbf_move_left_joystick(context, {0, -1}, 1200ms, 2s);
    pbf_move_left_joystick(context, {-1, -1}, 1700ms, 100ms);
    pbf_move_left_joystick(context, {0, 1}, 4000ms, 100ms);
    mash_until_black_screen(env, context, BUTTON_A);
    pbf_move_left_joystick(context, {1, 1}, 5500ms, 100ms);
    pbf_move_left_joystick(context, {0, -1}, 800ms, 100ms);
    pbf_move_left_joystick(context, {1, 0}, 9000ms, 100ms);
    pbf_mash_button(context, BUTTON_A, 35s);

    // route 2
    overworld_navigation(env, context, 1, 0, 15, true);
    overworld_navigation(env, context, -0.5, 1, 15, true);
    overworld_navigation(env, context, 1, 0, 15);
    overworld_navigation(env, context, -1, 1, 15);
    // align with pokeball on the ground, checkpoint candidate
    pbf_move_left_joystick(context, {-1, 0}, 100ms, 100ms);
    pbf_mash_button(context, BUTTON_A, 7s);
    // finish the rest of the trainers
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 1000ms);
    overworld_navigation(env, context, 0.8, 0.6, 15);
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 1000ms);
    overworld_navigation(env, context, 0, 1, 15);
    mash_until_black_screen_multi(env, context, 3, BUTTON_A);

    // rival battle, receive Dynamax Band
    pbf_move_left_joystick(context, {1, -1}, 3s, 100ms);
    pbf_move_left_joystick(context, {0, -1}, 12s, 100ms);
    pbf_mash_button(context, BUTTON_A, 5s);
    pbf_move_left_joystick(context, {-1, 0.1}, 600ms, 100ms);
    mash_until_black_screen_multi(env, context, 6, BUTTON_A);

    save_from_overworld(env, context);
}

void AutoStory::segment_05(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_move_left_joystick(context, {1, -1}, 3s, 100ms);
    pbf_move_left_joystick(context, {0, -1}, 8s, 100ms);
    pbf_mash_button(context, BUTTON_A, 20s);

    overworld_navigation(env, context, 0, -1, 15);
    overworld_navigation(env, context, -1, -1, 15);
    overworld_navigation(env, context, 0, -1, 15);
    overworld_navigation(env, context, -1, 0, 15);
    overworld_navigation(env, context, 0, -1, 15);

    // align with the bridge, checkpoint candidate
    overworld_navigation(env, context, -1, -0.3, 15);

    pbf_move_left_joystick(context, {0, 1}, 300ms, 100ms);
    pbf_move_left_joystick(context, {-1, -0.3}, 7s, 100ms);
    pbf_move_left_joystick(context, {0, -1}, 6s, 100ms);
    pbf_mash_button(context, BUTTON_A, 12s);
    pbf_move_left_joystick(context, {0, -1}, 15s, 100ms);
    pbf_move_left_joystick(context, {-1, 1}, 5s, 100ms);
    mash_until_black_screen(env, context);
    pbf_move_left_joystick(context, {0, 1}, 5s, 100ms);
    mash_until_black_screen(env, context, BUTTON_A);
    pbf_mash_button(context, BUTTON_A, 12s);

    save_from_overworld(env, context);
}

void AutoStory::segment_06(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_move_left_joystick(context, {0, -1}, 100ms, 100ms);
    pbf_move_left_joystick(context, {1, 0}, 1700ms, 100ms);
    pbf_move_left_joystick(context, {-0.3, -1}, 1s, 6s);
    pbf_move_left_joystick(context, {-0.2, 1}, 5s, 100ms);
    mash_until_black_screen_multi(env, context, 2, BUTTON_A);

    // deposit team, switch first attack to a status move
    pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
    pbf_press_dpad(context, DPAD_LEFT, 1s, 100ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_button(context, BUTTON_A, 160ms, 1500ms);
    pbf_press_button(context, BUTTON_R, 160ms, 2s);
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    pbf_press_button(context, BUTTON_Y, 100ms, 100ms);
    pbf_press_button(context, BUTTON_Y, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_LEFT, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_DOWN, 100ms, 100ms);
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_DOWN, 2s, 100ms);
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    pbf_press_button(context, BUTTON_B, 100ms, 2s);

    pbf_press_button(context, BUTTON_A, 160ms, 500ms);
    pbf_press_button(context, BUTTON_A, 160ms, 2000ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    pbf_press_button(context, BUTTON_A, 160ms, 500ms);
    pbf_press_button(context, BUTTON_A, 160ms, 500ms);
    pbf_press_dpad(context, DPAD_DOWN,100ms, 100ms);
    pbf_press_button(context, BUTTON_A, 160ms, 500ms);
    pbf_mash_button(context, BUTTON_B, 6s);

    // navigate to Onyx
    pbf_move_left_joystick(context, {0, 1}, 1000ms, 100ms);
    pbf_move_right_joystick(context, {-1, 0}, 400ms, 100ms);
    pbf_move_left_joystick(context, {0, 1}, 6000ms, 100ms);
    pbf_mash_button(context, BUTTON_B, 1s);

    // whistle until it attacks the player
    context.wait_for_all_requests();
    env.log("Wait for a pokemon to attack you.", COLOR_PURPLE);
    {
        StandardBattleMenuWatcher fight_detector(false);
        int result = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                while (true){
                    pbf_press_button(context, BUTTON_LCLICK, 80ms, 1000ms);
                }
            },
            {{fight_detector}}
        );
        if (result == 0){
            env.log("New fight detected.", COLOR_PURPLE);
            pbf_mash_button(context, BUTTON_B, 1000ms);
        }
    }
    // faint and teleport to the other side of the wild area
    mash_until_black_screen_multi(env, context, 3, BUTTON_A);
    pbf_mash_button(context, BUTTON_A, 10s);

    save_from_overworld(env, context);
}

void AutoStory::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    segment_00(env, context);
    segment_01(env, context);
    segment_02(env, context);
    segment_03(env, context);
    segment_04(env, context);
    segment_05(env, context);
    segment_06(env, context);
}



}
}
}
