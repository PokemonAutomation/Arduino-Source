/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP_EggRoutines.h"
#include "PokemonBDSP_EggHatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


EggHatcher_Descriptor::EggHatcher_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:EggHatcher",
        STRING_POKEMON + " BDSP", "Egg Hatcher",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/EggHatcher.md",
        "Hatch eggs from boxes.",
        FeedbackType::OPTIONAL_, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


EggHatcher::EggHatcher(const EggHatcher_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
//    , SHORTCUT("<b>Bike Shortcut:</b>")
    , BOXES_TO_HATCH(
        "<b>Boxes to Hatch:</b>",
        3
    )
    , SAVE_AND_RESET(
        "<b>Save and Reset:</b><br>After hatching a box, save the game and reset. This will recover from game crashes.",
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
    , SAFETY_TIME0(
        "<b>Safety Time:</b><br>Additional time added to the spinning.",
        "10 * TICKS_PER_SECOND"
    )
    , HATCH_DELAY(
        "<b>Hatch Delay:</b><br>Total animation time for hatching 5 eggs when there are no shinies.",
        "105 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
//    PA_ADD_OPTION(SHORTCUT);
    PA_ADD_OPTION(BOXES_TO_HATCH);
    PA_ADD_OPTION(STEPS_TO_HATCH);
    PA_ADD_OPTION(SAVE_AND_RESET);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SAFETY_TIME0);
    PA_ADD_OPTION(HATCH_DELAY);
}



struct EggHatcher::Stats : public StatsTracker{
    Stats()
        : m_batches(m_stats["Batches"])
    {
        m_display_order.emplace_back("Batches");
    }
    std::atomic<uint64_t>& m_batches;
};
std::unique_ptr<StatsTracker> EggHatcher::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



void EggHatcher::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    uint16_t INCUBATION_TIME = (uint16_t)((1258.5 + 4.05 * STEPS_TO_HATCH) * 1.05);
    uint16_t TOTAL_DELAY = INCUBATION_TIME + SAFETY_TIME0 + HATCH_DELAY;

    //  Connect the controller.
    pbf_move_right_joystick(env.console, 0, 255, 10, 0);

    uint8_t batches = BOXES_TO_HATCH * 6;
    uint8_t column = 0;
    for (uint8_t c = 0; c < batches; c++){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS_UPDATE,
            env.program_info(),
            "",
            stats.to_str()
        );

        if (c == 0){
            withdraw_1st_column_from_overworld(env.console);
        }else if (column < 5 || !SAVE_AND_RESET){
            swap_party(env.console, column);
            column++;
            if (column == 6){
                column = 0;
            }
        }else{
            deposit_party_to_column(env.console, 5);
            pbf_press_button(env.console, BUTTON_R, 20, GameSettings::instance().BOX_CHANGE_DELAY_0);
            box_to_overworld(env.console);
            save_game(env.console);
            pbf_press_button(env.console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, context, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            withdraw_1st_column_from_overworld(env.console);
            column = 0;
        }

        pbf_move_left_joystick(env.console, 0, 255, 125, 0);
        egg_spin_with_A(env.console, TOTAL_DELAY);

        stats.m_batches++;
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
    GO_HOME_WHEN_DONE.run_end_of_program(env.console);
}





}
}
}
