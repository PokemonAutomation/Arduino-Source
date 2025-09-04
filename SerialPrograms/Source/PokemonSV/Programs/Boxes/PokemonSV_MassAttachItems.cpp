/*  Mass Attach Items
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxAttach.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV_MassAttachItems.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


MassAttachItems_Descriptor::MassAttachItems_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:MassAttachItems",
        STRING_POKEMON + " SV", "Mass Attach Items",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/MassAttachItems.md",
        "Mass attach items to " + STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct MassAttachItems_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_boxes(m_stats["Boxes"])
        , m_attached(m_stats["Attached"])
//        , m_replaced(m_stats["Replaced"])
        , m_empty(m_stats["Empty Slots"])
        , m_eggs(m_stats["Eggs"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Boxes");
        m_display_order.emplace_back("Attached");
//        m_display_order.emplace_back("Replaced");
        m_display_order.emplace_back("Empty Slots");
        m_display_order.emplace_back("Eggs");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_boxes;
    std::atomic<uint64_t>& m_attached;
//    std::atomic<uint64_t>& m_replaced;
    std::atomic<uint64_t>& m_empty;
    std::atomic<uint64_t>& m_eggs;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> MassAttachItems_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



MassAttachItems::MassAttachItems()
    : GO_HOME_WHEN_DONE(false)
    , ITEM_CATEGORY(
        "<b>Item Category:</b>",
        {
            {ItemCategory::Medicines,   "medicine",     "Medicines"},
            {ItemCategory::PokeBalls,   "balls",        "Pok\u00e9 Balls"},
            {ItemCategory::BattleItems, "battle",       "Battle Items"},
            {ItemCategory::Berries,     "berries",      "Berries"},
            {ItemCategory::OtherItems,  "other",        "Other Items"},
            {ItemCategory::TMs,         "tms",          "TMs"},
            {ItemCategory::Treasures,   "treasures",    "Treasures"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        ItemCategory::PokeBalls
    )
    , BOXES(
        "<b>Number of Boxes to Attach:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 0, 32
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(ITEM_CATEGORY);
    PA_ADD_OPTION(BOXES);
    PA_ADD_OPTION(NOTIFICATIONS);
}




void MassAttachItems::attach_one(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    MassAttachItems_Descriptor::Stats& stats = env.current_stats<MassAttachItems_Descriptor::Stats>();

    env.log("Selecting " + STRING_POKEMON + "...");

    SomethingInBoxSlotDetector sth_in_box_detector(COLOR_RED);
    BoxCurrentEggDetector egg_detector;
    VideoOverlaySet overlays(env.console.overlay());
    sth_in_box_detector.make_overlays(overlays);
    egg_detector.make_overlays(overlays);
    context.wait_for_all_requests();

    VideoSnapshot screen = env.console.video().snapshot();

    if (!sth_in_box_detector.detect(screen)){
        stats.m_empty++;
        return;
    }

    if (egg_detector.detect(screen)){
        stats.m_eggs++;
        return;
    }

    try{
        size_t errors = 0;
        attach_item_from_box(env.program_info(), env.console, context, (size_t)ITEM_CATEGORY.get(), errors);
        stats.m_attached++;
        stats.m_errors += errors;
    }catch (OperationFailedException&){
        stats.m_errors++;
        throw;
    }
}
void MassAttachItems::attach_box(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    for (uint8_t row = 0; row < 5; row++){
        for (uint8_t j_col = 0; j_col < 6; j_col++){
            // Go through slots in a Z-shape pattern
            uint8_t col = (row % 2 == 0 ? j_col : 5 - j_col);
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, row, col);
            attach_one(box_detector, env, context);
            env.update_stats();
        }
    }
}



void MassAttachItems::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    MassAttachItems_Descriptor::Stats& stats = env.current_stats<MassAttachItems_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    BoxDetector box_detector;
    VideoOverlaySet overlays(env.console.overlay());
    box_detector.make_overlays(overlays);

    for (uint8_t box = 0; box < BOXES; box++){
        if (box > 0){
            move_to_right_box(context);
        }
        context.wait_for_all_requests();

        env.update_stats();

        attach_box(box_detector, env, context);

        stats.m_boxes++;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
