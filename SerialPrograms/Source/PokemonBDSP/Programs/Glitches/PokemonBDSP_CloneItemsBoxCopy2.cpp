/*  Clone Items (Box Swap Method 2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BoxRelease.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP_CloneItemsBoxCopy2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


CloneItemsBoxCopy2_Descriptor::CloneItemsBoxCopy2_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:CloneItemsBoxCopy2",
        STRING_POKEMON + " BDSP", "Clone Items (Box Copy 2)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/CloneItemsBoxCopy2.md",
        "With the menu glitch active, clone entire boxes of items at a time. "
        "<font color=\"red\">(The menu glitch can only be activated on version 1.1.0 - 1.1.3.)</font>",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct CloneItemsBoxCopy2_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_boxes(m_stats["Boxes Cloned"])
        , m_errors(m_stats["Errors"])
//        , m_resets(m_stats["Resets"])
    {
        m_display_order.emplace_back("Boxes Cloned");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
//        m_display_order.emplace_back("Resets");
    }
    std::atomic<uint64_t>& m_boxes;
    std::atomic<uint64_t>& m_errors;
//    std::atomic<uint64_t>& m_resets;
};
std::unique_ptr<StatsTracker> CloneItemsBoxCopy2_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


CloneItemsBoxCopy2::CloneItemsBoxCopy2()
    : GO_HOME_WHEN_DONE(false)
    , BOXES(
        "<b>Boxes to Clone:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        999, 0, 999
    )
    , RELEASE(
        "<b>Release the pokemon after cloning them:</b>"
        "Beware, if set to false, the pokemons will be stored in the subsequent boxes. Make sure you have enough empty boxes.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(BOXES);
    PA_ADD_OPTION(RELEASE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void CloneItemsBoxCopy2::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    CloneItemsBoxCopy2_Descriptor::Stats& stats = env.current_stats<CloneItemsBoxCopy2_Descriptor::Stats>();

//    uint16_t MENU_TO_POKEMON_DELAY = GameSettings::instance().MENU_TO_POKEMON_DELAY;
//    uint16_t POKEMON_TO_BOX_DELAY = GameSettings::instance().POKEMON_TO_BOX_DELAY0;
//    uint16_t OVERWORLD_TO_MENU_DELAY = GameSettings::instance().OVERWORLD_TO_MENU_DELAY;
//    uint16_t MENU_TO_OVERWORLD_DELAY = GameSettings::instance().MENU_TO_OVERWORLD_DELAY;
    Milliseconds BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY0;
    Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;
    Milliseconds BOX_CHANGE_DELAY = GameSettings::instance().BOX_CHANGE_DELAY0;
//    uint16_t BOX_TO_POKEMON_DELAY = GameSettings::instance().BOX_TO_POKEMON_DELAY;
//    uint16_t POKEMON_TO_MENU_DELAY = GameSettings::instance().POKEMON_TO_MENU_DELAY;

    //  Connect the controller.
    pbf_mash_button(context, BUTTON_RCLICK, 50);

    //  Enter box system.
    menu_to_box(context);

    context.wait_for_all_requests();
    VideoSnapshot expected = env.console.video().snapshot();
    ImageMatchWatcher matcher(std::move(expected.frame), {0.02, 0.25, 0.96, 0.73}, 20);

    for (uint16_t box = 0; box < BOXES; box++){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        //  Select 1st mon.
        pbf_press_button(context, BUTTON_ZL, 20, 50);

        //  Enter box system again.
        overworld_to_box(context);

#if 0
        //  Detach all items.
        pbf_press_button(context, BUTTON_X, 20, 50);
        detach_box(context, BOX_SCROLL_DELAY);

        //  Back to previous menu.
        box_to_overworld(context);

        //  View Summary
        pbf_move_right_joystick(context, 128, 255, 20, 10);
        pbf_press_button(context, BUTTON_ZL, 20, 250);

        //  Back out.
        pbf_press_button(context, BUTTON_B, 20, 230);

#else
        //  Move entire box to a new box.
        pbf_press_button(context, BUTTON_Y, 20, 50);
        pbf_press_button(context, BUTTON_Y, 20, 50);
        pbf_press_button(context, BUTTON_ZL, 160ms, BOX_PICKUP_DROP_DELAY);
        for (size_t c = 0; c < 10; c++){
            pbf_move_right_joystick(context, 255, 128, 5, 3);
            pbf_move_right_joystick(context, 128, 255, 5, 3);
        }
        pbf_press_button(context, BUTTON_ZL, 160ms, BOX_PICKUP_DROP_DELAY);
        int box_offset = RELEASE ? 1 : 1 + box;
        for (int i = 0; i < box_offset; ++i){
            pbf_press_button(context, BUTTON_R, 160ms, BOX_CHANGE_DELAY);
        }
        pbf_press_button(context, BUTTON_ZL, 160ms, BOX_PICKUP_DROP_DELAY);

        //  Back to previous menu.
        box_to_overworld(context);

        //  View Summary
        pbf_move_right_joystick(context, 128, 255, 20, 10);
        pbf_press_button(context, BUTTON_ZL, 20, 250);

        //  Back out.
        pbf_press_button(context, BUTTON_B, 20, 230);

        if (RELEASE){
            //  Release the cloned box.
            pbf_press_button(context, BUTTON_R, 160ms, BOX_CHANGE_DELAY);
            release_box(context, BOX_SCROLL_DELAY);
            pbf_press_button(context, BUTTON_L, 160ms, BOX_CHANGE_DELAY);

            //  Move cursor back to starting position.
            pbf_move_right_joystick(context, 128, 255, 160ms, BOX_SCROLL_DELAY);
            pbf_move_right_joystick(context, 128, 255, 160ms, BOX_SCROLL_DELAY);
            pbf_move_right_joystick(context, 128, 255, 160ms, BOX_SCROLL_DELAY);
            pbf_move_right_joystick(context, 255, 128, 160ms, BOX_SCROLL_DELAY);
            pbf_move_right_joystick(context, 255, 128, 160ms, BOX_SCROLL_DELAY);
        }
#endif

        context.wait_for_all_requests();
        context.wait_for(std::chrono::milliseconds(500));
        if (!matcher.detect(env.console.video().snapshot())){
            stats.m_errors++;
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to return to starting position. Something is wrong.",
                env.console
            );
        }

        stats.m_boxes++;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}







}
}
}
