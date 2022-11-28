/*  Mass Release
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_BoxDetection.h"
#include "PokemonSV_MassRelease.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


MassRelease_Descriptor::MassRelease_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:MassRelease",
        STRING_POKEMON + " SV", "Mass Release",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/MassRelease.md",
        "Farm items from Tera raids.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct MassRelease_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_boxes(m_stats["Boxes Cleared"])
        , m_released(m_stats["Released"])
        , m_empty(m_stats["Empty Slots"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Boxes Cleared");
        m_display_order.emplace_back("Released");
        m_display_order.emplace_back("Empty Slots");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& m_boxes;
    std::atomic<uint64_t>& m_released;
    std::atomic<uint64_t>& m_empty;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> MassRelease_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



MassRelease::MassRelease()
    : GO_HOME_WHEN_DONE(false)
    , BOXES_TO_RELEASE(
        "<b>Number of Boxes to Release:</b>",
        LockWhileRunning::UNLOCKED,
        2, 0, 32
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(BOXES_TO_RELEASE);
    PA_ADD_OPTION(NOTIFICATIONS);
}







void MassRelease::release_one(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    MassRelease_Descriptor::Stats& stats = env.current_stats<MassRelease_Descriptor::Stats>();

    context.wait_for_all_requests();

    env.log("Selecting " + STRING_POKEMON + "...");
    {
        BoxSelectWatcher selected(COLOR_RED);
        if (!selected.exists(env.console.video().snapshot())){
            stats.m_empty++;
            env.log("Slot is empty.");
            return;
        }
        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                pbf_press_button(context, BUTTON_A, 20, 230);
            },
            {selected}
        );
        if (ret < 0){
            stats.m_errors++;
            dump_image(
                env.console, env.program_info(),
                "SelectionArrowNotFound",
                env.console.video().snapshot()
            );
            throw OperationFailedException(env.logger(), "Unable to detect selection arrow.");
        }
        context.wait_for(std::chrono::milliseconds(100));
    }

    pbf_press_dpad(context, DPAD_UP, 20, 30);
    pbf_press_dpad(context, DPAD_UP, 20, 30);
    context.wait_for_all_requests();

    env.log("Releasing...");
    {
        PromptDialogWatcher confirm(COLOR_CYAN);
        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                pbf_press_button(context, BUTTON_A, 20, 230);
            },
            {confirm}
        );
        if (ret < 0){
            stats.m_errors++;
            dump_image(
                env.console, env.program_info(),
                "ConfirmationNotFound",
                env.console.video().snapshot()
            );
            throw OperationFailedException(env.logger(), "Release confirmation not found.");
        }
        context.wait_for(std::chrono::milliseconds(500));
    }

    pbf_press_dpad(context, DPAD_UP, 20, 30);
    context.wait_for_all_requests();

    env.log("Waiting for release to finish...");
    {
        SomethingInBoxSlotWatcher done_releasing(COLOR_RED, false);
        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                pbf_mash_button(context, BUTTON_A, 5 * TICKS_PER_SECOND);
            },
            {done_releasing}
        );
        if (ret < 0){
            stats.m_errors++;
            dump_image(
                env.console, env.program_info(),
                "ReleaseFailed",
                env.console.video().snapshot()
            );
            throw OperationFailedException(env.logger(), "Unable to release " + STRING_POKEMON + ".");
        }
        context.wait_for(std::chrono::milliseconds(100));
    }
    stats.m_released++;
}
void MassRelease::release_box(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    for (uint8_t row = 0; row < 5; row++){
        for (uint8_t col = 0; col < 6; col++){
            box_detector.move_cursor(env.console, context, BoxCursorLocation::SLOTS, row, col);
            release_one(box_detector, env, context);
            env.update_stats();
        }
    }
}



void MassRelease::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    MassRelease_Descriptor::Stats& stats = env.current_stats<MassRelease_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    BoxDetector box_detector;
    VideoOverlaySet overlays(env.console.overlay());
    box_detector.make_overlays(overlays);

    for (uint8_t box = 0; box < BOXES_TO_RELEASE; box++){
        env.update_stats();

        release_box(box_detector, env, context);

        pbf_press_button(context, BUTTON_R, 20, 105);
        stats.m_boxes++;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
