/*  Poke Jobs Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh_DateSpam-PokeJobsFarmer.h"

namespace PokemonAutomation
{
	namespace NintendoSwitch
	{
		namespace PokemonSwSh
		{

			PokeJobsFarmer_Descriptor::PokeJobsFarmer_Descriptor()
				: RunnableSwitchProgramDescriptor(
					  "PokemonSwSh:PokeJobsFarmer",
					  "Date Spam: Poke Jobs Farmer",
					  "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DateSpam-PokeJobsFarmer.md",
					  "Farm PokéJobs.",
					  FeedbackType::NONE,
					  PABotBaseLevel::PABOTBASE_12KB)
			{
			}

			PokeJobsFarmer::PokeJobsFarmer(const PokeJobsFarmer_Descriptor &descriptor)
				: SingleSwitchProgramInstance(descriptor),
				  SKIPS(
					  "<b>Maximum number of Poke Jobs completed:</b>",
					  1000),
				  MASH_B_DURATION(
					  "<b>Mash B for this long to exit the dialog:</b>",
					  "9 * TICKS_PER_SECOND"),
				  MENU_INDEX(
					  "<b>Index of Poke Jobs in Rotom menu:</b>",
					  3),
				  CONCURRENCY(
					  "<b>Number of concurrent Poke Jobs:</b>",
					  2)
			{
				PA_ADD_OPTION(SKIPS);
				PA_ADD_OPTION(MASH_B_DURATION);
				PA_ADD_OPTION(MENU_INDEX);
				PA_ADD_OPTION(CONCURRENCY);
			}

			void PokeJobsFarmer::program(SingleSwitchProgramEnvironment &env)
			{
				uint8_t year = MAX_YEAR;

				for (uint32_t c = 0; c < SKIPS; c++)
				{
					if (MAX_YEAR <= year)
					{
						// Roll back to 2001
						env.log("Rolling back!");
						pbf_press_button(env.console, BUTTON_B, 5, 5);
						pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);
						home_roll_date_enter_game_autorollback(env.console, &year);
						pbf_mash_button(env.console, BUTTON_B, MASH_B_DURATION);
					}

					for (uint16_t j = 1; j <= CONCURRENCY; j++)
					{
						env.log("Starting job #" + tostr_u_commas(j) + " in year " + tostr_u_commas(2000 + year));

						// Enter menu
						env.log("#### Entering menu");
						pbf_press_button(env.console, BUTTON_A, 10, 90);
						pbf_press_button(env.console, BUTTON_B, 10, 90);

						// Select PokéJobs entry
						env.log("#### Select PokéJobs entry");
						for (uint16_t i = 1; i < MENU_INDEX; i++)
						{
							pbf_press_dpad(env.console, DPAD_DOWN, 10, 10);
						}
						pbf_press_button(env.console, BUTTON_A, 10, 4 * TICKS_PER_SECOND); // Wait for animation to complete

						// Select first PokéJob
						env.log("#### Select first Poke Job");
						pbf_mash_button(env.console, BUTTON_A, 1 * TICKS_PER_SECOND); // Skip new job notification if any
						pbf_press_button(env.console, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
						pbf_press_button(env.console, BUTTON_A, 10, 3 * TICKS_PER_SECOND);

						// Select all Pokémons
						env.log("#### Select all");
						pbf_press_button(env.console, BUTTON_X, 10, 90);

						// Send to PokéJob
						env.log("#### Send to Poke Job");
						pbf_press_button(env.console, BUTTON_B, 10, 2 * TICKS_PER_SECOND);
						pbf_mash_button(env.console, BUTTON_A, 6 * TICKS_PER_SECOND); // Mash until animation starts

						// Wait for animation to end and exit PokéJobs
						env.log("#### Exit Poke Jobs");
						pbf_mash_button(env.console, BUTTON_B, MASH_B_DURATION);
					}

					env.log("Skipping one year...");

					// Go to home menu
					env.log("#### Go to Switch home menu");
					pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);

					// Skip frame
					env.log("#### Skip frame");
					home_roll_date_enter_game_autorollback(env.console, &year);
					pbf_mash_button(env.console, BUTTON_B, 1 * TICKS_PER_SECOND);

					for (uint16_t j = 1; j <= CONCURRENCY; j++)
					{
						env.log("Completing job #" + tostr_u_commas(j) + " in year " + tostr_u_commas(2000 + year));

						// Enter menu
						env.log("#### Entering menu");
						pbf_press_button(env.console, BUTTON_A, 10, 90);
						pbf_press_button(env.console, BUTTON_B, 10, 90);

						// Select Poké Jobs entry
						env.log("#### Select Poke Jobs entry");
						for (uint16_t i = 1; i < MENU_INDEX; i++)
						{
							pbf_press_dpad(env.console, DPAD_DOWN, 10, 10);
						}
						pbf_press_button(env.console, BUTTON_A, 10, 4 * TICKS_PER_SECOND); // Wait for animation to complete

						// Select first PokéJob
						env.log("#### Select first Poke Job");
						pbf_mash_button(env.console, BUTTON_A, 1 * TICKS_PER_SECOND);
						pbf_press_button(env.console, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
						pbf_press_button(env.console, BUTTON_A, 10, 10 * TICKS_PER_SECOND); // Wait for animation to complete

						// Skip through wall of text and exit
						env.log("#### Exit Poke Jobs");
						pbf_mash_button(env.console, BUTTON_B, MASH_B_DURATION);
					}
				}

				end_program_callback(env.console);
				end_program_loop(env.console);
			}

		}
	}
}
