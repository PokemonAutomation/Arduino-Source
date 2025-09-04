/*  Egg Autonomous
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_IvJudgeReader.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP_EggAutonomousState.h"
#include "PokemonBDSP_EggAutonomous.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;



EggAutonomous_Descriptor::EggAutonomous_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:EggAutonomous",
        STRING_POKEMON + " BDSP", "Egg Autonomous",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/EggAutonomous.md",
        "Automatically fetch+hatch eggs and keep all shinies.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
std::unique_ptr<StatsTracker> EggAutonomous_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new EggAutonomousStats());
}


EggAutonomous::EggAutonomous()
    : GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b><br>Required to read IVs.",
        PokemonSwSh::IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , SHORTCUT("<b>Bike Shortcut:</b>")
    , MAX_KEEPERS(
        "<b>Max Keepers:</b><br>Stop the program after keeping this many " + STRING_POKEMON + ". "
        "This number plus the number of " + STRING_POKEMON + " in your last box must not exceed 30. "
        "Otherwise, the program will break when that box is full.",
        LockMode::LOCK_WHILE_RUNNING,
        10, 1, 30
    )
    , TRAVEL_TIME_PER_FETCH0(
        "<b>Travel Time per Fetch:</b><br>Fetch an egg after traveling for this long.",
        LockMode::LOCK_WHILE_RUNNING,
        "20 s"
    )
    , NUM_EGGS_IN_COLUMN(
        "<b>Num Eggs in Column:</b><br>How many eggs already deposited in the first column in Box 1.",
        {
            {0, "0", "0"},
            {1, "1", "1"},
            {2, "2", "2"},
            {3, "3", "3"},
            {4, "4", "4"},
            {5, "5", "5"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , AUTO_SAVING(
        "<b>Auto-Saving:</b><br>Automatically save the game to recover from crashes and allow eggs to be unhatched.<br>"
        "(Unhatching eggs can be useful for obtaining breeding parents by rehatching a perfect egg in a game with a different language.)<br><br>"
        "To collect (unhatched) eggs with the desired stats, set this option to \"Save before every batch\". "
        "Then set the Action Table below to \"Stop Program\" on the desired stats. "
        "Once the program stops on the baby with the desired stats, you can manually reset the game and it will revert to an egg in your party.",
        {
            {AutoSave::NoAutoSave, "none", "No auto-saving. (No error/crash recovery.)"},
            {AutoSave::AfterStartAndKeep, "start-and-keep", "Save at beginning and after obtaining each baby that is kept. (Allows for error/crash recovery.)"},
            {AutoSave::EveryBatch, "every-batch", "Save before every batch. (Allows you to unhatch eggs.)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        AutoSave::AfterStartAndKeep
    )
    , FILTERS0(
        StatsHuntIvJudgeFilterTable_Label_Eggs,
        {
            .action = true,
            .shiny = true,
            .gender = true,
            .nature = true,
        }
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_NONSHINY_KEEP(
        "Non-Shiny Keep",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs"}
    )
    , NOTIFICATION_SHINY(
        "Shiny Hatch",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_NONSHINY_KEEP,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , SCROLL_TO_READ_DELAY0(
        "<b>Scroll to Read Delay:</b><br>Wait this long after scrolling in the box to read the " + STRING_POKEMON + "'s stats. "
        "Increase this if your video has high latency.",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
    )
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(SHORTCUT);
    PA_ADD_OPTION(MAX_KEEPERS);
    PA_ADD_OPTION(TRAVEL_TIME_PER_FETCH0);
    PA_ADD_OPTION(NUM_EGGS_IN_COLUMN);
    PA_ADD_OPTION(AUTO_SAVING);
    PA_ADD_OPTION(FILTERS0);
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SCROLL_TO_READ_DELAY0);
}





bool EggAutonomous::run_batch(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    EggAutonomousState& saved_state,
    EggAutonomousState& current_state
){
    env.update_stats();
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

    bool save = false;
    switch (AUTO_SAVING){
    case AutoSave::NoAutoSave:
        break;
    case AutoSave::AfterStartAndKeep:
        save = saved_state.babies_saved() != current_state.babies_saved();
        break;
    case AutoSave::EveryBatch:
        save = true;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid saving option.");
    }

    if (save){
        save_game(env.console, context);
        saved_state.set(current_state);
    }

    while (!current_state.overworld_detect_and_run_state());
    return current_state.process_batch();
}

void EggAutonomous::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    EggAutonomousStats& stats = env.current_stats<EggAutonomousStats>();
    env.update_stats();

    //  Connect the controller.
    pbf_move_right_joystick(context, 0, 255, 10, 0);

    //  Move to corner.
    pbf_move_left_joystick(context, 0, 255, 125, 0);

    EggAutonomousState current_state(
        env, env.console, context,
        stats,
        NOTIFICATION_NONSHINY_KEEP,
        NOTIFICATION_SHINY,
        SCROLL_TO_READ_DELAY0,
        LANGUAGE,
        SHORTCUT,
        TRAVEL_TIME_PER_FETCH0,
        FILTERS0,
        MAX_KEEPERS,
        static_cast<uint8_t>(NUM_EGGS_IN_COLUMN.current_value())
    );
    EggAutonomousState saved_state = current_state;

//    overworld_to_box(env, env.console);
//    current_state.withdraw_egg_column();
//    box_to_overworld(env, env.console);

    if (AUTO_SAVING == AutoSave::AfterStartAndKeep){
        save_game(env.console, context);
        saved_state.set(current_state);
    }

    size_t consecutive_failures = 0;
    while (current_state.babies_saved() < MAX_KEEPERS){
        try{
            if (run_batch(env, context, saved_state, current_state)){
                break;
            }
            consecutive_failures = 0;
        }catch (OperationFailedException& e){
            // If there is no auto save, then we shouldn't reset to game to lose previous progress.
            if (AUTO_SAVING == AutoSave::NoAutoSave){
                throw;
            }
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            consecutive_failures++;
            if (consecutive_failures >= 3){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed 3 batches in the row.",
                    env.console
                );
            }
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            current_state.set(saved_state);
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}








}
}
}
