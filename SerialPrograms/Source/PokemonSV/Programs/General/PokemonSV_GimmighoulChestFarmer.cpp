/*  Gimmighoul Chest Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV_GimmighoulChestFarmer.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSV {

using namespace Pokemon;

GimmighoulChestFarm_Descriptor::GimmighoulChestFarm_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:GimmighoulChestFarm",
        STRING_POKEMON + " SV", "Gimmighoul Chest Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/GimmighoulChestFarmer.md",
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
    assert_16_9_720p_min(env.logger(), env.console);

    GimmighoulChestFarm_Descriptor::Stats& stats = env.current_stats<GimmighoulChestFarm_Descriptor::Stats>();
    uint32_t c = 0;
    while(c < PP) {
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
            pbf_press_button(context, BUTTON_L, 50, 40);
            pbf_move_left_joystick(context, 128, 0, 120, 0);
        }

        //Close the game
        save_game_from_overworld(env.program_info(), env.console, context);
        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        context.wait_for_all_requests();
        close_game(context);

        //Date skip - in-game day cycle is 72 mins, so 2 hours is fastest way
        //This isn't perfect because 12 hour format but it works
        home_to_date_time(context, true, false);
        pbf_press_dpad(context, DPAD_DOWN, 10, 100);
        pbf_press_dpad(context, DPAD_DOWN, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_dpad(context, DPAD_RIGHT, 10, 100);
        pbf_press_dpad(context, DPAD_RIGHT, 10, 100);
        pbf_press_dpad(context, DPAD_RIGHT, 10, 100);
        pbf_press_dpad(context, DPAD_UP, 10, 100);
        pbf_press_dpad(context, DPAD_UP, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_A, 10, 100);
        pbf_press_button(context, BUTTON_HOME, 10, 90);

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

