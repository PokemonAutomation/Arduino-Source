/*  Gimmighoul Chest Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Inference/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV_GimmighoulChestFarm.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSV {

using namespace Pokemon;

    GimmighoulChestFarm_Descriptor::GimmighoulChestFarm_Descriptor()
        : SingleSwitchProgramDescriptor(
            "PokemonSV:GimmighoulChestFarm",
            STRING_POKEMON + " SV", "Gimmighoul Chest Farmer",
            "ComputerControl/blob/master/Wiki/Programs/PokemonSV/GimmighoulChestFarm.md",
            "Farm Chest Gimmighoul for coins.",
            FeedbackType::REQUIRED, true,
            PABotBaseLevel::PABOTBASE_12KB
        )
    {}

	struct GimmighoulChestFarm_Descriptor::Stats : public StatsTracker {
		Stats()
			: pokemon_fainted(m_stats["Chests farmed"])
			, resets(m_stats["Resets"])
			, errors(m_stats["Errors"])
		{
			m_display_order.emplace_back(Stat("Chests farmed"));
			m_display_order.emplace_back(Stat("Resets"));
			m_display_order.emplace_back(Stat("Errors"));
		}
		std::atomic<uint64_t>& pokemon_fainted;
		std::atomic<uint64_t>& resets;
		std::atomic<uint64_t>& errors;
	};
	std::unique_ptr<StatsTracker> GimmighoulChestFarm_Descriptor::make_stats() const {
		return std::unique_ptr<StatsTracker>(new Stats());
	}

    GimmighoulChestFarm::GimmighoulChestFarm()
        : PP(
            "<b>First Attack PP:</b><br>The amount of PP remaining on your lead's first attack.",
            LockWhileRunning::LOCKED,
            15
        )
        , GO_HOME_WHEN_DONE(false)
        , FIX_TIME_WHEN_DONE(
            "<b>Fix time when done:</b><br>Fix the time after the program finishes.",
            LockWhileRunning::LOCKED, false
        )
		, NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
        , NOTIFICATIONS({
			&NOTIFICATION_STATUS_UPDATE,
            &NOTIFICATION_PROGRAM_FINISH,
            &NOTIFICATION_ERROR_FATAL,
            })
    {
        PA_ADD_OPTION(PP);
        PA_ADD_OPTION(GO_HOME_WHEN_DONE);
        PA_ADD_OPTION(FIX_TIME_WHEN_DONE);
        PA_ADD_OPTION(NOTIFICATIONS);
    }

    void GimmighoulChestFarm::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
        //Start in game facing a Chest Gimmighoul spawn in a Watchtower
        //Your lead Pokemon must be able to kill Gimmighoul in one hit with its first move.
		//Pick a move with an accuracy of 100 and use a high leveled lead, Gimmighoul isn't hard to one-shot.
		//None of the pokemon in your party are able to level up or evolve
		//please use a fast attack for speed
		GimmighoulChestFarm_Descriptor::Stats& stats = env.current_stats<GimmighoulChestFarm_Descriptor::Stats>();
		uint32_t c = 0;
		while(c < PP) {
		//for (uint32_t c = 0; c < SKIPS; c++) {
            //Press A to enter battle, assuming there is a chest
            env.log("Fetch Attempts: " + tostr_u_commas(c));
            pbf_mash_button(context, BUTTON_A, 90);

			//Wait for the battle to load then check for battle menu, if there isn't a battle menu then no chest
			BattleMenuWatcher battle_menu(COLOR_YELLOW);
			int ret = wait_until(
				env.console, context,
				std::chrono::seconds(10),
				{ battle_menu }
			);

			if (ret == 0) {
				//Attack using your first move
				pbf_mash_button(context, BUTTON_A, 90);
				c++;
				context.wait_for_all_requests();
				OverworldWatcher overworld(COLOR_RED);
				int ret2 = wait_until(
					env.console, context,
					std::chrono::seconds(120),
					{ overworld }
				);
				if (ret2 != 0) {
					stats.errors++;
					env.update_stats();
					throw OperationFailedException(env.console, "Failed to return to Overworld after two minutes. Did your attack miss or fail to defeat Gimmighoul in one hit?");
				}
				stats.pokemon_fainted++;
				env.update_stats();
				send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

				//Walk forward since battles force you to jump back
				//Bumping into the wall seems to work to reset the position
				pbf_press_button(context, BUTTON_L, 50, 40);
				pbf_move_left_joystick(context, 128, 0, 280, 0);
			}
			//Save game
			save_game_from_overworld(env.console, context);
            //Save the game then close it
            //save_game_from_overworld(env.console, context);
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            context.wait_for_all_requests();
            close_game(context);
			
            //Date skip
            home_to_date_time(context, true, false);
            roll_date_forward_1(context, false);
            pbf_press_button(context, BUTTON_HOME, 10, 90);

            //Enter game
			stats.resets++;
			env.update_stats();
			send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
            reset_game_from_home(env, env.console, context, 5 * TICKS_PER_SECOND);
        }

        if (FIX_TIME_WHEN_DONE) {
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
            home_to_date_time(context, false, false);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
            pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
        }
        GO_HOME_WHEN_DONE.run_end_of_program(context);
        send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    }

}
}
}

