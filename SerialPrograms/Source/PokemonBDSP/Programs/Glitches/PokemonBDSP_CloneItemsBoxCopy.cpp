/*  Clone Items (Box Swap Method)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BoxRelease.h"
#include "PokemonBDSP/Programs/Eggs/PokemonBDSP_EggRoutines.h"
#include "PokemonBDSP_MenuOverlap.h"
#include "PokemonBDSP_CloneItemsBoxCopy.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


CloneItemsBoxCopy_Descriptor::CloneItemsBoxCopy_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:CloneItemsBoxCopy",
        STRING_POKEMON + " BDSP", "Clone Items (Box Copy)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/CloneItemsBoxCopy.md",
        "Clone 30 items at a time using the menu overlap glitch via the entire box cloning method. "
        "<font color=\"red\">(This requires game versions 1.1.0 - 1.1.1. The glitch it relies on was patched in v1.1.2.)</font>",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

CloneItemsBoxCopy::CloneItemsBoxCopy(const CloneItemsBoxCopy_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , BOXES(
        "<b>Boxes to Clone:</b>",
        999, 0, 999
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(BOXES);
    PA_ADD_OPTION(NOTIFICATIONS);
}


struct CloneItemsBoxCopy::Stats : public StatsTracker{
    Stats()
        : m_boxes(m_stats["Boxes Cloned"])
        , m_errors(m_stats["Errors"])
//        , m_resets(m_stats["Resets"])
    {
        m_display_order.emplace_back("Boxes Cloned");
        m_display_order.emplace_back("Errors", true);
//        m_display_order.emplace_back("Resets");
    }
    std::atomic<uint64_t>& m_boxes;
    std::atomic<uint64_t>& m_errors;
//    std::atomic<uint64_t>& m_resets;
};
std::unique_ptr<StatsTracker> CloneItemsBoxCopy::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

void CloneItemsBoxCopy::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    uint16_t MENU_TO_POKEMON_DELAY = GameSettings::instance().MENU_TO_POKEMON_DELAY;
    uint16_t POKEMON_TO_BOX_DELAY = GameSettings::instance().POKEMON_TO_BOX_DELAY0;
//    uint16_t OVERWORLD_TO_MENU_DELAY = GameSettings::instance().OVERWORLD_TO_MENU_DELAY;
//    uint16_t MENU_TO_OVERWORLD_DELAY = GameSettings::instance().MENU_TO_OVERWORLD_DELAY;
    uint16_t BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY;
    uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY_0;
    uint16_t BOX_CHANGE_DELAY = GameSettings::instance().BOX_CHANGE_DELAY_0;
//    uint16_t BOX_TO_POKEMON_DELAY = GameSettings::instance().BOX_TO_POKEMON_DELAY;
//    uint16_t POKEMON_TO_MENU_DELAY = GameSettings::instance().POKEMON_TO_MENU_DELAY;

    //  Connect the controller.
    pbf_mash_button(env.console, BUTTON_B, 50);

    size_t consecutive_failures = 0;
    for (uint16_t box = 0; box < BOXES; box++){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS_UPDATE,
            env.program_info(),
            "",
            stats.to_str()
        );

        QImage start = activate_menu_overlap_from_overworld(env.console);
        if (start.isNull()){
            stats.m_errors++;
            consecutive_failures++;
            if (consecutive_failures >= 3){
                PA_THROW_StringException("Failed to activate menu overlap glitch 3 times in the row.");
            }
            pbf_mash_button(env.console, BUTTON_B, 10 * TICKS_PER_SECOND);
            continue;
        }
        consecutive_failures = 0;

        //  Enter box system.
        pbf_mash_button(env.console, BUTTON_ZL, 30);
        if (MENU_TO_POKEMON_DELAY > 30){
            pbf_wait(env.console, MENU_TO_POKEMON_DELAY - 30);
        }
        pbf_press_button(env.console, BUTTON_R, 20, POKEMON_TO_BOX_DELAY);

        //  Move to Battle.
        pbf_press_button(env.console, BUTTON_ZL, 20, 50);

        //  Enter box system again.
        overworld_to_box(env.console);

        //  Move entire box to next box.
        pbf_press_button(env.console, BUTTON_Y, 20, 50);
        pbf_press_button(env.console, BUTTON_Y, 20, 50);
        pbf_press_button(env.console, BUTTON_ZL, 20, BOX_PICKUP_DROP_DELAY);
        for (size_t c = 0; c < 10; c++){
            pbf_move_right_joystick(env.console, 255, 128, 5, 3);
            pbf_move_right_joystick(env.console, 128, 255, 5, 3);
        }
        pbf_press_button(env.console, BUTTON_ZL, 20, BOX_PICKUP_DROP_DELAY);
        pbf_press_button(env.console, BUTTON_R, 20, BOX_CHANGE_DELAY);
        pbf_press_button(env.console, BUTTON_ZL, 20, BOX_PICKUP_DROP_DELAY);

        //  Back to previous menu.
        box_to_overworld(env.console);

        //  View Summary
        pbf_move_right_joystick(env.console, 128, 255, 20, 10);
        pbf_press_button(env.console, BUTTON_ZL, 20, 250);

        //  Back all the way out to the overworld and clear glitch.
        back_out_to_overworld(env, env.console, start);

        //  Release the cloned box.
        overworld_to_box(env.console);
        pbf_press_button(env.console, BUTTON_R, 20, BOX_CHANGE_DELAY);
        release_box(env.console, BOX_SCROLL_DELAY);
        pbf_press_button(env.console, BUTTON_L, 20, BOX_CHANGE_DELAY);

        //  Back all the way out to the overworld and clear glitch.
        back_out_to_overworld(env, env.console, start);

        stats.m_boxes++;
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
