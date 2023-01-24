/*  Poke Jobs Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh_DateSpam-PokeJobsFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;



PokeJobsFarmer_Descriptor::PokeJobsFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
          "PokemonSwSh:PokeJobsFarmer",
          STRING_POKEMON + " SwSh",
          "Date Spam - " + STRING_POKEJOB + "s Farmer",
          "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DateSpam-PokeJobsFarmer.md",
          "Farm " + STRING_POKEJOB + "s.",
          FeedbackType::NONE,
          AllowCommandsWhenRunning::DISABLE_COMMANDS,
          PABotBaseLevel::PABOTBASE_12KB)
{
}

PokeJobsFarmer::PokeJobsFarmer()
    : SKIPS(
        "<b>Number of days:</b>",
        LockWhileRunning::LOCKED,
        200
    )
    , CONCURRENCY(
        "<b>Number of concurrent " + STRING_POKEJOB + "s per day:</b>",
        LockWhileRunning::LOCKED,
        2
    )
    , MENU_INDEX(
        "<b>Index of " + STRING_POKEJOB + "s in Rotom menu:</b>",
        LockWhileRunning::LOCKED,
        3
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , MASH_B_DURATION(
        "<b>Mash B for this long upon completion of " + STRING_POKEJOB + ":</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "8 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(CONCURRENCY);
    PA_ADD_OPTION(MENU_INDEX);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(MASH_B_DURATION);
}

static void enter_jobs(BotBaseContext& context, uint16_t index){
    // Enter menu
    pbf_press_button(context, BUTTON_A, 10, 90);
    pbf_press_button(context, BUTTON_B, 10, 90);
    // Select entry
    for (uint16_t i = 1; i < index; i++)
    {
        pbf_press_dpad(context, DPAD_DOWN, 10, 10);
    }
    pbf_press_button(context, BUTTON_A, 10, 5 * TICKS_PER_SECOND); // Wait for animation to complete
}

void PokeJobsFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    uint8_t year = MAX_YEAR;

    // Play it safe in case some menu is open
    pbf_mash_button(context, BUTTON_B, MASH_B_DURATION);

    for (uint32_t c = 0; c < SKIPS; c++)
    {
        if (MAX_YEAR <= year)
        {
            // Roll back to 2001
            env.log("Rolling back!");
            pbf_press_button(context, BUTTON_B, 5, 5);
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);
            home_roll_date_enter_game_autorollback(env.console, context, year);
            pbf_mash_button(context, BUTTON_B, MASH_B_DURATION);

            // Get rid of new jobs notification by entering Poke Jobs and leaving immediately
            enter_jobs(context, MENU_INDEX);
            pbf_mash_button(context, BUTTON_B, 5 * TICKS_PER_SECOND);
        }

        // Start new jobs
        for (uint16_t j = 1; j <= CONCURRENCY; j++)
        {
            env.log("Starting job #" + tostr_u_commas(j) + " in year " + tostr_u_commas(2000 + year));

            // Enter Poke Jobs
            enter_jobs(context, MENU_INDEX);

            // Select first Poke Job
            env.log("#### Select first " + STRING_POKEJOB);
            pbf_press_button(context, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A, 10, 3 * TICKS_PER_SECOND);

            // Select all Pokemons
            env.log("#### Select all");
            pbf_press_button(context, BUTTON_X, 10, 90);

            // Send to Poke Job
            env.log("#### Send to " + STRING_POKEJOB);
            pbf_press_button(context, BUTTON_B, 10, 2 * TICKS_PER_SECOND);
            pbf_mash_button(context, BUTTON_A, 6 * TICKS_PER_SECOND); // Mash until animation starts

            // Wait for animation to end and exit Poke Jobs
            env.log("#### Exit " + STRING_POKEJOB + "s");
            pbf_mash_button(context, BUTTON_B, 5 * TICKS_PER_SECOND);
        }

        env.log("Skipping one year...");

        // Go to home menu
        env.log("#### Go to Switch home menu");
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);

        // Skip frame
        env.log("#### Skip frame");
        home_roll_date_enter_game_autorollback(env.console, context, year);
        pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);

        // Get rid of new jobs notification by entering Poke Jobs and leaving immediately
        enter_jobs(context, MENU_INDEX);
        pbf_mash_button(context, BUTTON_B, 5 * TICKS_PER_SECOND);

        // Complete jobs
        for (uint16_t j = 1; j <= CONCURRENCY; j++)
        {
            env.log("Completing job #" + tostr_u_commas(j) + " in year " + tostr_u_commas(2000 + year));

            // Enter Poke Jobs
            enter_jobs(context, MENU_INDEX);

            // Select first Poke Job
            env.log("#### Select first " + STRING_POKEJOB);
            pbf_press_button(context, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A, 10, 10 * TICKS_PER_SECOND); // Wait for animation to complete

            // Skip through wall of text and exit
            env.log("#### Exit " + STRING_POKEJOB + "s");
            pbf_mash_button(context, BUTTON_B, MASH_B_DURATION);
        }
    }
}



}
}
}
