/*  Gimmighoul Chest Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_GimmighoulChestFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

GimmighoulChestFarmer_Descriptor::GimmighoulChestFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:GimmighoulChestFarmer",
        STRING_POKEMON + " SV", "Gimmighoul Chest Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/GimmighoulChestFarmer.md",
        "Farm Chest Gimmighoul for coins.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

struct GimmighoulChestFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : pokemon_fainted(m_stats["Chests farmed"])
        , wild_interrupts(m_stats["Wild interrupts"])
        , resets(m_stats["Resets"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Chests farmed");
        m_display_order.emplace_back("Wild interrupts");
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& pokemon_fainted;
    std::atomic<uint64_t>& wild_interrupts;
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> GimmighoulChestFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

GimmighoulChestFarmer::GimmighoulChestFarmer()
    : PP(
        "<b>First Attack PP:</b><br>The amount of PP remaining on your lead's first attack.",
        LockMode::LOCK_WHILE_RUNNING,
        15
    )
    , START_LOCATION(
        "<b>Start Location:</b><br>The start location of your character.",
        {
            {StartLocation::FlyPoint, "fly-point", "Fly Point - East Province (Area One) Watchtower"},
            {StartLocation::InFrontOfChest, "in-front-of-chest", "In front of chest"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StartLocation::FlyPoint
    )
    , GO_HOME_WHEN_DONE(false)
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time when Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(PP);
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void GimmighoulChestFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    if (START_LOCATION == StartLocation::FlyPoint){
        //Set starting position by flying - must fly to East Province (Area One) Watchtower, do not move from fly point
        open_map_from_overworld(env.program_info(), env.console, context);
        fly_to_overworld_from_map(env.program_info(), env.console, context);
        pbf_move_left_joystick(context, 0, 0, 158, 0);
        pbf_press_button(context, BUTTON_L, 50, 40);
        pbf_move_left_joystick(context, 128, 0, 100, 0);
        //Climb ladder
        pbf_press_button(context, BUTTON_L, 50, 40);
        pbf_move_left_joystick(context, 128, 0, 2350, 0);
        pbf_press_button(context, BUTTON_L, 50, 40);
        pbf_wait(context, 100);
        context.wait_for_all_requests();
        //Walk into the wall
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        context.wait_for_all_requests();
        //Press A in case there's already a chest
        //The remaining commands will run harmlessly during the battle intro if there is a chest
        pbf_press_button(context, BUTTON_A, 50, 40);
        //Turn back
        pbf_move_left_joystick(context, 128, 255, 60, 100);
        context.wait_for_all_requests();
        //Position toward chest
        pbf_move_left_joystick(context, 128, 0, 30, 0);
        context.wait_for_all_requests();
    }
    //else assuming player is positioned correctly in front of the chest

    GimmighoulChestFarmer_Descriptor::Stats& stats = env.current_stats<GimmighoulChestFarmer_Descriptor::Stats>();
    uint32_t c = 0;
    while(c < PP){

        //  Press A to enter battle, assuming there is a chest
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        pbf_mash_button(context, BUTTON_A, 90);

        //Wait for the battle to load then check for battle menu, if there isn't a battle menu then no chest
        NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(10),
            { battle_menu }
        );

        if (ret == 0){
            //  Attack using your first move
            pbf_mash_button(context, BUTTON_A, 90);
            c++;
            context.wait_for_all_requests();
            OverworldWatcher overworld(env.console, COLOR_RED);
            int ret2 = wait_until(
                env.console, context,
                std::chrono::seconds(120),
                { overworld }
            );
            if (ret2 != 0){
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    env.console, ErrorReport::SEND_ERROR_REPORT,
                    "Failed to return to Overworld after two minutes. Did your attack miss or fail to defeat Gimmighoul in one hit?"
                );
            }
            stats.pokemon_fainted++;
            env.update_stats();
            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

            //Set starting position by flying - move map cursor
            open_map_from_overworld(env.program_info(), env.console, context);
            pbf_press_button(context, BUTTON_ZR, 50, 40);
            pbf_move_left_joystick(context, 48, 192, 10, 0);
            fly_to_overworld_from_map(env.program_info(), env.console, context);
            pbf_move_left_joystick(context, 0, 0, 158, 0);
            pbf_press_button(context, BUTTON_L, 50, 40);
            pbf_move_left_joystick(context, 128, 0, 100, 0);
            //Climb ladder
            pbf_press_button(context, BUTTON_L, 50, 40);
            pbf_move_left_joystick(context, 128, 0, 2350, 0);
            pbf_press_button(context, BUTTON_L, 50, 40);
            pbf_wait(context, 100);
            context.wait_for_all_requests();
            //Walk into the wall
            pbf_move_left_joystick(context, 128, 0, 200, 100);
            context.wait_for_all_requests();
            //Turn back
            pbf_move_left_joystick(context, 128, 255, 60, 100);
            context.wait_for_all_requests();
            //Position toward chest
            pbf_move_left_joystick(context, 128, 0, 30, 0);
            context.wait_for_all_requests();

            //Check for tauros interrupt before pressing A - reset position if there was one
            ret = wait_until(
                env.console, context,
                std::chrono::seconds(1),
                { battle_menu }
            );
            if (ret == 0){
                pbf_mash_button(context, BUTTON_A, 90);
                c++;
                context.wait_for_all_requests();
                ret2 = wait_until(
                    env.console, context,
                    std::chrono::seconds(120),
                    { overworld }
                );
                if (ret2 != 0){
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        env.console, ErrorReport::SEND_ERROR_REPORT,
                        "Failed to return to Overworld after two minutes."
                    );
                }
                //Don't move map cursor this time
                open_map_from_overworld(env.program_info(), env.console, context);
                fly_to_overworld_from_map(env.program_info(), env.console, context);
                pbf_move_left_joystick(context, 0, 0, 158, 0);
                pbf_press_button(context, BUTTON_L, 50, 40);
                pbf_move_left_joystick(context, 128, 0, 100, 0);
                //Climb ladder
                pbf_press_button(context, BUTTON_L, 50, 40);
                pbf_move_left_joystick(context, 128, 0, 2350, 0);
                pbf_press_button(context, BUTTON_L, 50, 40);
                pbf_wait(context, 100);
                context.wait_for_all_requests();
                //Walk into the wall
                pbf_move_left_joystick(context, 128, 0, 200, 100);
                context.wait_for_all_requests();
                //Turn back
                pbf_move_left_joystick(context, 128, 255, 60, 100);
                context.wait_for_all_requests();
                //Position toward chest
                pbf_move_left_joystick(context, 128, 0, 30, 0);
                context.wait_for_all_requests();

                stats.wild_interrupts++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
            }

        }

        //  Save the game
        save_game_from_overworld(env.program_info(), env.console, context);
        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);

        //  Date skip - in-game day cycle is 72 mins, so 2 hours is fastest way
        //  This isn't perfect because 12 hour format but it works
        home_to_date_time(context, true, false);
        ssf_press_button(context, BUTTON_A, 20, 10);
        ssf_issue_scroll(context, DPAD_RIGHT, 0);
        ssf_press_button(context, BUTTON_A, 2);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
        ssf_issue_scroll(context, SSF_SCROLL_UP, 3);
        ssf_issue_scroll(context, SSF_SCROLL_UP, 3);
        ssf_issue_scroll(context, DPAD_RIGHT, 0);
        ssf_press_button(context, BUTTON_A, 2);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
        ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
        ssf_press_button(context, BUTTON_A, 20, 10);
        pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);

        stats.resets++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        reset_game_from_home(env.program_info(), env.console, context, 5 * TICKS_PER_SECOND);
    }

    if (FIX_TIME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
        home_to_date_time(context, false, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
        resume_game_from_home(env.console, context);
    }
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

