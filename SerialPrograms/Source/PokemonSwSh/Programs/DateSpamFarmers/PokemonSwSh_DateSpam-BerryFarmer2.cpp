/*  Berry Farmer 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Sounds/PokemonSwSh_BerryTreeRustlingSoundDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_DateSpam-BerryFarmer2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


BerryFarmer2_Descriptor::BerryFarmer2_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:BerryFarmer2",
        STRING_POKEMON + " SwSh", "Date Spam - Berry Farmer 2",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DateSpam-BerryFarmer2.md",
        "Farm berries using Feedback.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



BerryFarmer2::BerryFarmer2()
    : REQUIRES_AUDIO(
        html_color_text(
            "<font size=4><b>Rustling detection uses sound. Make sure you have the correct audio input set.</b></font>",
            COLOR_BLUE
        )
    )
    , FETCH_ATTEMPTS(
        "<b>Number of Fetch Attempts:</b>",
        100000
    )
    , SAVE_ITERATIONS(
        "<b>Save Every this Many Fetches:</b><br>(zero disables saving): ",
        0
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        TICKS_PER_SECOND,
        "10 * TICKS_PER_SECOND"
    )
    , START_BATTLE_DELAY(
        "<b>Start Battle Delay:</b><br>After a battle is detected, wait this long to flee.",
        TICKS_PER_SECOND,
        "15 * TICKS_PER_SECOND"
    )
    , RUSTLING_INTERVAL(
        "<b>Rustling Interval:</b><br>How much time between two rustling sounds has to pass to be considered slow rustling in ms.",
        1350
    )
    , RUSTLING_TIMEOUT(
        "<b>Rustling Timeout:</b><br>Wait this many ticks to detect rustling.",
        TICKS_PER_SECOND,
        "400"
    )
    , SECONDARY_ATTEMPT_MASH_TIME(
        "<b>Secondary attempt mash time:</b><br>Mash ZL this many ticks for secondary fetch attempts.",
        TICKS_PER_SECOND,
        "240"
    )
{
    PA_ADD_OPTION(REQUIRES_AUDIO);
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(FETCH_ATTEMPTS);
    PA_ADD_OPTION(SAVE_ITERATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
    PA_ADD_OPTION(START_BATTLE_DELAY);
    PA_ADD_OPTION(RUSTLING_INTERVAL);
    PA_ADD_OPTION(RUSTLING_TIMEOUT);
    PA_ADD_OPTION(SECONDARY_ATTEMPT_MASH_TIME);
}


BerryFarmer2::Rustling BerryFarmer2::check_rustling(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    // wait some time in order to not detect rustling from previous fetch attempt
    pbf_wait(context, 80);
    context.wait_for_all_requests();

    BerryTreeRustlingSoundDetector initial_rustling_detector(env.console.logger(), env.console, [&](float error_coefficient) -> bool {
        //  Warning: This callback will be run from a different thread than this function.
        return true;
        });

    BerryTreeRustlingSoundDetector secondary_rustling_detector(env.console.logger(), env.console, [&](float error_coefficient) -> bool {
        //  Warning: This callback will be run from a different thread than this function.
        return true;
        });

    StandardBattleMenuWatcher battle_menu_detector(false);
    StartBattleWatcher start_battle_detector;

    Rustling result = Rustling::No;
    int ret = run_until(
        env.console, context,
        [&](BotBaseContext& context) {
            pbf_wait(context, RUSTLING_TIMEOUT);
            context.wait_for_all_requests();
        },
        { {initial_rustling_detector}, {battle_menu_detector}, {start_battle_detector} }
        );

    if (ret == 0) {
        env.console.log("BerryFarmer: Initial Rustling detected.");
        WallClock initial_rustling_time = current_time();

        result = Rustling::Slow;
        
        int ret1 = run_until(
            env.console, context,
            [&](BotBaseContext& context) {
                pbf_wait(context, RUSTLING_TIMEOUT);
                context.wait_for_all_requests();
            },
            { {secondary_rustling_detector} }
            );

        if (ret1 == 0) {
            env.console.log("BerryFarmer: Secondary Rustling detected.");
            WallClock secondary_rustling_time = current_time();
            if (std::chrono::duration_cast<Milliseconds>(secondary_rustling_time - initial_rustling_time).count() <= RUSTLING_INTERVAL) {
                result = Rustling::Fast;
            }
        }

    }
    else if (ret == 1) {
        env.console.log("BerryFarmer: Battle Menu detected!");
        result = Rustling::Battle;
    } 
    else if (ret == 2) {
        env.console.log("BerryFarmer: Battle Start detected.");
        // waiting here so we can be sure the battle is fully underway so escaping is possible
        pbf_wait(context, START_BATTLE_DELAY);
        context.wait_for_all_requests();
        result = Rustling::Battle;
    }
    else {
        result = Rustling::No;
    }
        
    context.wait_for_all_requests();
    return result;
}

uint32_t BerryFarmer2::do_secondary_attempts(SingleSwitchProgramEnvironment& env, BotBaseContext& context, Rustling rustling) {
    uint8_t no_rustling = (rustling == Rustling::No) ? 1 : 0;
    Rustling current_rustling = rustling;
    uint32_t attempts = 0;

    while ((current_rustling == Rustling::Slow || current_rustling == Rustling::No) && no_rustling < 3) { 
        /* Slow or No rustling, not in Battle! */
        pbf_mash_button(context, BUTTON_ZL, 240);
        pbf_mash_button(context, BUTTON_B, 10);
        attempts++;

        current_rustling = check_rustling(env, context);

        if (current_rustling == Rustling::No) {
            no_rustling++;
        }
    }
    /* Fast rustling, in Battle or too many times No rustling */
    if (no_rustling >= 3) {
        return attempts;
    }
    if (current_rustling == Rustling::Fast) {
        // this is the last tree interaction for this time skip
        pbf_mash_button(context, BUTTON_ZL, SECONDARY_ATTEMPT_MASH_TIME);
        pbf_mash_button(context, BUTTON_B, 10);
        attempts++;
        current_rustling = check_rustling(env, context);
    }
    if (current_rustling == Rustling::Battle) {
        pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
        env.console.log("BerryFarmer: Running away!");
        run_away(env.console, context, EXIT_BATTLE_TIMEOUT);
        context.wait_for_all_requests();
    }
    return attempts;
}

void BerryFarmer2::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(context);
    }
    else {
        pbf_press_button(context, BUTTON_B, 5, 5);
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);
    }

    uint8_t year = MAX_YEAR;
    uint16_t save_count = 0;
    uint32_t c = 0;
    while (c < FETCH_ATTEMPTS) {
        uint16_t iteration_attempts = 1;
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        
        home_roll_date_enter_game_autorollback(context, &year);
        // Interact with the tree
        pbf_mash_button(context, BUTTON_ZL, 375);
        pbf_mash_button(context, BUTTON_B, 10);

        // Rustling after the first fetch attempt
        Rustling current_rustling = check_rustling(env, context);
        
        switch (current_rustling)
        {
        case Rustling::Battle:
            pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);
            run_away(env.console, context, EXIT_BATTLE_TIMEOUT);
            break;
        case Rustling::Fast:
            // Do nothing -> stop current tree session
            break;
        case Rustling::No:
        case Rustling::Slow:
            uint32_t secondary_attempts = do_secondary_attempts(env, context, current_rustling);
            iteration_attempts += secondary_attempts;
            break;
        }

        // end tree session
        pbf_mash_button(context, BUTTON_B, iteration_attempts > 1 ? 800 : 600);

        c += iteration_attempts;

        if (SAVE_ITERATIONS != 0) {
            save_count += iteration_attempts;
            if (save_count >= SAVE_ITERATIONS){
                save_count = 0;
                pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
                pbf_press_button(context, BUTTON_R, 20, 2 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_ZL, 20, 3 * TICKS_PER_SECOND);
            }
        }

        // Tap HOME and quickly spam B. The B spamming ensures that we don't
        // accidentally update the system if the system update window pops up.
        pbf_press_button(context, BUTTON_HOME, 10, 5);
        pbf_mash_button(context, BUTTON_B, GameSettings::instance().GAME_TO_HOME_DELAY_FAST - 15);
    }

}
}



}
}

