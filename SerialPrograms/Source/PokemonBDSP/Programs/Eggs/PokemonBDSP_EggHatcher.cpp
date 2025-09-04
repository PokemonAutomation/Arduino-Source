/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP_EggRoutines.h"
#include "PokemonBDSP_EggHatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


EggHatcher_Descriptor::EggHatcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:EggHatcher",
        STRING_POKEMON + " BDSP", "Egg Hatcher",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/EggHatcher.md",
        "Hatch eggs from boxes.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::OPTIONAL_,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct EggHatcher_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_batches(m_stats["Batches"])
    {
        m_display_order.emplace_back("Batches");
    }
    std::atomic<uint64_t>& m_batches;
};
std::unique_ptr<StatsTracker> EggHatcher_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


EggHatcher::EggHatcher()
    : GO_HOME_WHEN_DONE(false)
//    , SHORTCUT("<b>Bike Shortcut:</b>")
    , BOXES_TO_HATCH(
        "<b>Boxes to Hatch:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        3
    )
    , SAVE_AND_RESET(
        "<b>Save and Reset:</b><br>After hatching a box, save the game and reset. This will recover from game crashes.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , SAFETY_TIME1(
        "<b>Safety Time:</b><br>Additional time added to the spinning.",
        LockMode::LOCK_WHILE_RUNNING,
        "10000 ms"
    )
    , HATCH_DELAY0(
        "<b>Hatch Delay:</b><br>Total animation time for hatching 5 eggs when there are no shinies.",
        LockMode::LOCK_WHILE_RUNNING,
        "105 s"
    )
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
//    PA_ADD_OPTION(SHORTCUT);
    PA_ADD_OPTION(BOXES_TO_HATCH);
    PA_ADD_OPTION(STEPS_TO_HATCH);
    PA_ADD_OPTION(SAVE_AND_RESET);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SAFETY_TIME1);
    PA_ADD_OPTION(HATCH_DELAY0);
}




void EggHatcher::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    EggHatcher_Descriptor::Stats& stats = env.current_stats<EggHatcher_Descriptor::Stats>();

    Milliseconds INCUBATION_TIME = (uint16_t)((1258.5 + 4.05 * STEPS_TO_HATCH) * 1.05) * 8ms;
    Milliseconds TOTAL_DELAY = INCUBATION_TIME + SAFETY_TIME1.get() + HATCH_DELAY0.get();

    //  Connect the controller.
    pbf_move_right_joystick(context, 0, 255, 10, 0);

    uint8_t batches = BOXES_TO_HATCH * 6;
    uint8_t column = 0;
    for (uint8_t c = 0; c < batches; c++){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        if (c == 0){
            withdraw_1st_column_from_overworld(context);
        }else if (column < 5 || !SAVE_AND_RESET){
            swap_party(context, column);
            column++;
            if (column == 6){
                column = 0;
            }
        }else{
            deposit_party_to_column(context, 5);
            pbf_press_button(context, BUTTON_R, 160ms, GameSettings::instance().BOX_CHANGE_DELAY0);
            box_to_overworld(context);
            save_game(context);
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            withdraw_1st_column_from_overworld(context);
            column = 0;
        }

        pbf_move_left_joystick(context, 0, 255, 125, 0);
        egg_spin_with_A(context, TOTAL_DELAY);

        stats.m_batches++;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}





}
}
}
