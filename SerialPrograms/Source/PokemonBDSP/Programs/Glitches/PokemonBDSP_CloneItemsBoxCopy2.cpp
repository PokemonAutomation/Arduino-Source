/*  Clone Items (Box Swap Method 2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BoxRelease.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/Eggs/PokemonBDSP_EggRoutines.h"
#include "PokemonBDSP_CloneItemsBoxCopy2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


CloneItemsBoxCopy2_Descriptor::CloneItemsBoxCopy2_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:CloneItemsBoxCopy2",
        STRING_POKEMON + " BDSP", "Clone Items (Box Copy 2)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/CloneItemsBoxCopy2.md",
        "With the menu glitch active, clone entire boxes of items at a time. "
        "<font color=\"red\">(The menu glitch can only be activated on version 1.1.0 - 1.1.3.)</font>",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

CloneItemsBoxCopy2::CloneItemsBoxCopy2(const CloneItemsBoxCopy2_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , BOXES(
        "<b>Boxes to Clone:</b>",
        999, 0, 999
    )
    , RELEASE(
        "<b>Release the pokemon after cloning them:</b>"
        "Beware, if set to false, the pokemons will be stored in the subsequent boxes. Make sure you have enough empty boxes.",
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


struct CloneItemsBoxCopy2::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> CloneItemsBoxCopy2::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

void CloneItemsBoxCopy2::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.current_stats<Stats>();

//    uint16_t MENU_TO_POKEMON_DELAY = GameSettings::instance().MENU_TO_POKEMON_DELAY;
//    uint16_t POKEMON_TO_BOX_DELAY = GameSettings::instance().POKEMON_TO_BOX_DELAY0;
//    uint16_t OVERWORLD_TO_MENU_DELAY = GameSettings::instance().OVERWORLD_TO_MENU_DELAY;
//    uint16_t MENU_TO_OVERWORLD_DELAY = GameSettings::instance().MENU_TO_OVERWORLD_DELAY;
    uint16_t BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY;
    uint16_t BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY_0;
    uint16_t BOX_CHANGE_DELAY = GameSettings::instance().BOX_CHANGE_DELAY_0;
//    uint16_t BOX_TO_POKEMON_DELAY = GameSettings::instance().BOX_TO_POKEMON_DELAY;
//    uint16_t POKEMON_TO_MENU_DELAY = GameSettings::instance().POKEMON_TO_MENU_DELAY;

    //  Connect the controller.
    pbf_mash_button(context, BUTTON_RCLICK, 50);

    //  Enter box system.
    menu_to_box(context);

    context.wait_for_all_requests();
    std::shared_ptr<const ImageRGB32> expected = std::make_shared<const ImageRGB32>(env.console.video().snapshot());
    ImageMatchWatcher matcher(std::move(expected), {0.02, 0.25, 0.96, 0.73}, 20);

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
        pbf_press_button(context, BUTTON_ZL, 20, BOX_PICKUP_DROP_DELAY);
        for (size_t c = 0; c < 10; c++){
            pbf_move_right_joystick(context, 255, 128, 5, 3);
            pbf_move_right_joystick(context, 128, 255, 5, 3);
        }
        pbf_press_button(context, BUTTON_ZL, 20, BOX_PICKUP_DROP_DELAY);
        int box_offset = RELEASE ? 1 : 1 + box;
        for (int i = 0; i < box_offset; ++i){
            pbf_press_button(context, BUTTON_R, 20, BOX_CHANGE_DELAY);
        }
        pbf_press_button(context, BUTTON_ZL, 20, BOX_PICKUP_DROP_DELAY);

        //  Back to previous menu.
        box_to_overworld(context);

        //  View Summary
        pbf_move_right_joystick(context, 128, 255, 20, 10);
        pbf_press_button(context, BUTTON_ZL, 20, 250);

        //  Back out.
        pbf_press_button(context, BUTTON_B, 20, 230);

        if (RELEASE){
            //  Release the cloned box.
            pbf_press_button(context, BUTTON_R, 20, BOX_CHANGE_DELAY);
            release_box(context, BOX_SCROLL_DELAY);
            pbf_press_button(context, BUTTON_L, 20, BOX_CHANGE_DELAY);

            //  Move cursor back to starting position.
            pbf_move_right_joystick(context, 128, 255, 20, BOX_SCROLL_DELAY);
            pbf_move_right_joystick(context, 128, 255, 20, BOX_SCROLL_DELAY);
            pbf_move_right_joystick(context, 128, 255, 20, BOX_SCROLL_DELAY);
            pbf_move_right_joystick(context, 255, 128, 20, BOX_SCROLL_DELAY);
            pbf_move_right_joystick(context, 255, 128, 20, BOX_SCROLL_DELAY);
        }
#endif

        context.wait_for_all_requests();
        context.wait_for(std::chrono::milliseconds(500));
        QImage current = env.console.video().snapshot();
        if (!matcher.detect(current)){
            stats.m_errors++;
            throw OperationFailedException(env.console, "Failed to return to starting position. Something is wrong.");
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
