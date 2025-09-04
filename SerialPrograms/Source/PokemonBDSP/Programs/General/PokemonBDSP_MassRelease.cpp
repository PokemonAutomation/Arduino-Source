/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BoxRelease.h"
#include "PokemonBDSP_MassRelease.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


MassRelease_Descriptor::MassRelease_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:MassRelease",
        STRING_POKEMON + " BDSP", "Mass Release",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/MassRelease.md",
        "Mass release boxes of " + STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct MassRelease_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_boxes_released(m_stats["Boxes Released"])
    {
        m_display_order.emplace_back("Boxes Released");
    }
    std::atomic<uint64_t>& m_boxes_released;
};
std::unique_ptr<StatsTracker> MassRelease_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



MassRelease::MassRelease()
    : GO_HOME_WHEN_DONE(false)
    , BOXES_TO_RELEASE(
        "<b>Number of Boxes to Release:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        2, 0, 40
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(BOXES_TO_RELEASE);
    PA_ADD_OPTION(NOTIFICATIONS);
}







void MassRelease::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    MassRelease_Descriptor::Stats& stats = env.current_stats<MassRelease_Descriptor::Stats>();
    env.update_stats();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    Milliseconds box_scroll_delay = GameSettings::instance().BOX_SCROLL_DELAY0;
    Milliseconds box_change_delay = GameSettings::instance().BOX_CHANGE_DELAY0;

    if (BOXES_TO_RELEASE > 0){
        env.update_stats();
        release_box(context, box_scroll_delay);
        stats.m_boxes_released++;
        for (uint8_t box = 1; box < BOXES_TO_RELEASE; box++){
            env.update_stats();
            pbf_press_dpad(context, DPAD_DOWN, 160ms, box_scroll_delay);
            pbf_press_dpad(context, DPAD_DOWN, 160ms, box_scroll_delay);
            pbf_press_dpad(context, DPAD_DOWN, 160ms, box_scroll_delay);
            pbf_press_dpad(context, DPAD_RIGHT, 160ms, box_scroll_delay);
            pbf_press_dpad(context, DPAD_RIGHT, 160ms, box_scroll_delay);
            pbf_wait(context, 50);
            pbf_press_button(context, BUTTON_R, 160ms, box_change_delay);
            release_box(context, box_scroll_delay);
            stats.m_boxes_released++;
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}





}
}
}
