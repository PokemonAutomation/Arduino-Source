/*  Gimmighoul Chest Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
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
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
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
    , ADDITIONAL_BATTLE_WAIT_TIME0(
        "<b>Additional Battle Wait Time:</b><br>Increase this if you are timing out when entering battle.",
        LockMode::LOCK_WHILE_RUNNING,
        "10000 ms"
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
    PA_ADD_OPTION(ADDITIONAL_BATTLE_WAIT_TIME0);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void GimmighoulChestFarmer::navigate_to_gimmi(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    //Cursor is already in position
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
}

void GimmighoulChestFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);
    assert_16_9_720p_min(env.logger(), env.console);

    GimmighoulChestFarmer_Descriptor::Stats& stats = env.current_stats<GimmighoulChestFarmer_Descriptor::Stats>();

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

    uint32_t c = 0;
    while(c < PP){
        //  Press A to enter battle, assuming there is a chest
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        pbf_mash_button(context, BUTTON_A, 125);
        pbf_wait(context, 125); //Wait extra to make sure the overworld map vanishes
        context.wait_for_all_requests();

        OverworldWatcher battleStarting(env.console, COLOR_RED);
        NormalBattleMenuWatcher battle_detected(COLOR_RED);
        int retOverworld = wait_until(
            env.console, context,
            std::chrono::seconds(5),
            {battleStarting, battle_detected}
        );
        if (retOverworld != 0) {
            //Wait for the battle to load then check for battle menu, if there isn't a battle menu then no chest
            NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
            int ret = wait_until(
                env.console, context,
                ADDITIONAL_BATTLE_WAIT_TIME0,
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
                        ErrorReport::SEND_ERROR_REPORT,
                        "Failed to return to Overworld after two minutes. Did your attack miss or fail to defeat Gimmighoul in one hit?",
                        env.console
                    );
                }
                stats.pokemon_fainted++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

                //Set starting position by flying - move map cursor
                open_map_from_overworld(env.program_info(), env.console, context);
                pbf_press_button(context, BUTTON_ZR, 50, 40);
                pbf_move_left_joystick(context, 48, 192, 10, 0);
                navigate_to_gimmi(env, context);

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
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to return to Overworld after two minutes.",
                            env.console
                        );
                    }
                    //Don't move map cursor this time
                    open_map_from_overworld(env.program_info(), env.console, context);
                    navigate_to_gimmi(env, context);

                    stats.wild_interrupts++;
                    env.update_stats();
                    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                }
            }
        }

        //  Save the game
        save_game_from_overworld(env.program_info(), env.console, context);
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);

        //  Date skip - in-game day cycle is 72 mins, so 2 hours is fastest way
        //  This isn't perfect because 12 hour format but it works
        home_to_date_time(env.console, context, true);
        ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
        ssf_issue_scroll_ptv(context, DPAD_RIGHT, 0ms);
        ssf_press_button_ptv(context, BUTTON_A, 16ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
        ssf_issue_scroll_ptv(context, DPAD_RIGHT, 0ms);
        ssf_press_button_ptv(context, BUTTON_A, 16ms);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
        ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
        ssf_press_button_ptv(context, BUTTON_A, 160ms, 80ms);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);

        stats.resets++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        reset_game_from_home(env.program_info(), env.console, context, 5 * TICKS_PER_SECOND);
    }

    if (FIX_TIME_WHEN_DONE){
        go_home(env.console, context);
        home_to_date_time(env.console, context, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(env.console, context);
    }
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

