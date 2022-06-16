/*  Egg Autonomous
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP_EggFeedback.h"
#include "PokemonBDSP_EggAutonomousState.h"
#include "PokemonBDSP_EggAutonomous.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
using namespace Pokemon;



EggAutonomous_Descriptor::EggAutonomous_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:EggAutonomous",
        STRING_POKEMON + " BDSP", "Egg Autonomous",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/EggAutonomous.md",
        "Automatically fetch+hatch eggs and keep all shinies.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


EggAutonomous::EggAutonomous(const EggAutonomous_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b><br>Required to read IVs.",
        IVCheckerReader::instance().languages(),
        false
    )
    , SHORTCUT("<b>Bike Shortcut:</b>")
    , MAX_KEEPERS(
        "<b>Max Keepers:</b><br>Stop the program after keeping this many " + STRING_POKEMON + ". "
        "This number plus the number of " + STRING_POKEMON + " in your last box must not exceed 30. "
        "Otherwise, the program will break when that box is full.",
        10, 1, 30
    )
    , TRAVEL_TIME_PER_FETCH(
        "<b>Travel Time per Fetch:</b><br>Fetch an egg after traveling for this long.",
        "20 * TICKS_PER_SECOND"
    )
    , NUM_EGGS_IN_COLUMN(
        "<b>Num Eggs in Column:</b><br>How many eggs already deposited in the first column in Box 1.",
        0, 0, 5
    )
    , AUTO_SAVING(
        "<b>Auto-Saving:</b><br>Automatically save the game to recover from crashes and allow eggs to be unhatched.<br>"
        "(Unhatching eggs can be useful for obtaining breeding parents by rehatching a perfect egg in a game with a different language.)<br><br>"
        "To collect (unhatched) eggs with the desired stats, set this option to \"Save after every batch\". "
        "Then set the Action Table below to \"Stop Program\" on the desired stats. "
        "Once the program stops on the baby with the desired stats, you can manually reset the game and it will revert to an egg in your party.",
        {
            "No auto-saving. (No error/crash recovery.)",
            "Save at beginning and after obtaining each baby that is kept. (Allows for error/crash recovery.)",
            "Save after every batch. (Allows you to unhatch eggs.)",
        },
        1
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
    , SCROLL_TO_READ_DELAY(
        "<b>Scroll to Read Delay:</b><br>Wait this long after scrolling in the box to read the " + STRING_POKEMON + "'s stats. "
        "Increase this if your video has high latency.",
        "125"
    )
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(SHORTCUT);
    PA_ADD_OPTION(MAX_KEEPERS);
    PA_ADD_OPTION(TRAVEL_TIME_PER_FETCH);
    PA_ADD_OPTION(NUM_EGGS_IN_COLUMN);
    PA_ADD_OPTION(AUTO_SAVING);
    PA_ADD_OPTION(FILTERS);
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SCROLL_TO_READ_DELAY);
}


std::unique_ptr<StatsTracker> EggAutonomous::make_stats() const{
    return std::unique_ptr<StatsTracker>(new EggAutonomousStats());
//    return nullptr;
}



bool EggAutonomous::run_batch(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    EggAutonomousState& saved_state,
    EggAutonomousState& current_state
){
    env.update_stats();
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

    bool save = false;
    switch (AUTO_SAVING){
    case 0:
        save = false;
        break;
    case 1:
        save = saved_state.babies_saved() != current_state.babies_saved();
        break;
    case 2:
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

void EggAutonomous::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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
        NOTIFICATION_ERROR_RECOVERABLE,
        SCROLL_TO_READ_DELAY,
        LANGUAGE,
        SHORTCUT,
        TRAVEL_TIME_PER_FETCH,
        FILTERS,
        MAX_KEEPERS,
        NUM_EGGS_IN_COLUMN
    );
    EggAutonomousState saved_state = current_state;

//    overworld_to_box(env, env.console);
//    current_state.withdraw_egg_column();
//    box_to_overworld(env, env.console);

    if (AUTO_SAVING == 1){
        save_game(env.console, context);
        saved_state.set(current_state);
    }

    size_t consecutive_failures = 0;
    while (current_state.babies_saved() < MAX_KEEPERS){
        if (AUTO_SAVING == 0){
            if (run_batch(env, context, saved_state, current_state)){
                break;
            }
            continue;
        }
        try{
            if (run_batch(env, context, saved_state, current_state)){
                break;
            }
            consecutive_failures = 0;
        }catch (OperationFailedException&){
            consecutive_failures++;
            if (consecutive_failures >= 3){
                throw OperationFailedException(env.console, "Failed 3 batches in the row.");
            }
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, context, true);
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
