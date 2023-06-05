/*  Size Checker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV_SizeChecker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


SizeChecker_Descriptor::SizeChecker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:SizeChecker",
        STRING_POKEMON + " SV", "SizeChecker",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/SizeChecker.md",
        "Check boxes of Pokémon for size marks.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct SizeChecker_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_boxes(m_stats["Boxes Checked"])
        , m_checked(m_stats["Checked"])
        , m_empty(m_stats["Empty Slots"])
        , m_eggs(m_stats["Eggs"])
    {
        m_display_order.emplace_back("Boxes Checked");
        m_display_order.emplace_back("Checked");
        m_display_order.emplace_back("Empty Slots");
        m_display_order.emplace_back("Eggs");
    }
    std::atomic<uint64_t>& m_boxes;
    std::atomic<uint64_t>& m_checked;
    std::atomic<uint64_t>& m_empty;
    std::atomic<uint64_t>& m_eggs;
};
std::unique_ptr<StatsTracker> SizeChecker_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



SizeChecker::SizeChecker()
    : GO_HOME_WHEN_DONE(false)
    , BOXES_TO_CHECK(
        "<b>Number of Boxes to Check:</b>",
        LockWhileRunning::LOCKED,
        2, 1, 32
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(BOXES_TO_CHECK);
    PA_ADD_OPTION(NOTIFICATIONS);
}




void SizeChecker::enter_check_mode(ConsoleHandle& console, BotBaseContext& context){
    WallClock start = current_time();

    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "enter_check_mode(): Failed to enter box mode after 5 minutes.",
                true
            );
        }
        OverworldWatcher overworld(COLOR_CYAN);
        AdvanceDialogWatcher dialog(COLOR_GREEN);
        GradientArrowWatcher prompt(COLOR_YELLOW, GradientArrowType::RIGHT, {0.72, 0.55, 0.05, 0.08});
        BoxWatcher box(COLOR_BLUE);
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {overworld, dialog, prompt, box}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            console.log("Detected overworld.");
            pbf_press_button(context, BUTTON_A, 20, 5);
            continue;
        case 1:
            console.log("Detected dialog box.");
            pbf_press_button(context, BUTTON_A, 20, 5);
            continue;
            // TODO: Detect if mark was given and update stats.
        case 2:
            console.log("Detected prompt.");
            pbf_press_button(context, BUTTON_A, 20, 5);
            continue;
        case 3:
            console.log("Detected box mode.", COLOR_ORANGE);
            return;

        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "enter_check_mode(): No recognized state after 60 seconds.",
                true
            );
        }
    }

}



void SizeChecker::exit_check_mode(ConsoleHandle& console, BotBaseContext& context){
    WallClock start = current_time();

    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "exit_check_mode(): Failed to exit box mode after 5 minutes.",
                true
            );
        }

        OverworldWatcher overworld(COLOR_CYAN);
        AdvanceDialogWatcher dialog(COLOR_GREEN);
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {overworld, dialog}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            console.log("Detected overworld.");
            return;
        case 1:
            console.log("Detected dialog box.");
            pbf_press_button(context, BUTTON_A, 20, 5);
            continue;

        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "exit_check_mode(): No recognized state after 60 seconds.",
                true
            );
        }

    }

}



void SizeChecker::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    SizeChecker_Descriptor::Stats& stats = env.current_stats<SizeChecker_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    // Loop through boxes.
    for (uint8_t box = 0; box < BOXES_TO_CHECK; box++){
        enter_check_mode(env.console, context);
        // Default state on box entry does not highlight selection, move cursor so it does.
        pbf_press_dpad(context, DPAD_LEFT, 5, 100);
        context.wait_for_all_requests();

        if (box > 0){
            move_to_right_box(context);
        }
        context.wait_for_all_requests();

        // Loop through the rows and columns.
        for (uint8_t row = 0; row < 5; row++){
            for (uint8_t col = 0; col < 6; col++){

                move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, row, col);

                SomethingInBoxSlotDetector sth_in_box_detector(COLOR_RED);
                BoxCurrentEggDetector egg_detector;
                VideoOverlaySet overlays(env.console.overlay());
                sth_in_box_detector.make_overlays(overlays);
                egg_detector.make_overlays(overlays);
                context.wait_for_all_requests();

                VideoSnapshot screen = env.console.video().snapshot();

                if (!sth_in_box_detector.detect(screen)){
                    env.console.log("Detected empty cell.");
                    stats.m_empty++;
                    env.update_stats();
                    continue;
                }

                if (egg_detector.detect(screen)){
                    env.console.log("Detected egg in cell.");
                    stats.m_eggs++;
                    env.update_stats();
                    continue;
                }

                // Initiate size checking prompt.
                pbf_press_button(context, BUTTON_A, 20, 20);

                // If last cell not empty.
                if (row == 4 && col == 5){
                    exit_check_mode(env.console, context);
                } else {
                    enter_check_mode(env.console, context);
                    // Default state on box entry does not highlight selection, move cursor so it does.
                    pbf_press_dpad(context, DPAD_LEFT, 5, 100);
                }

                context.wait_for_all_requests();

                stats.m_checked++;
                env.update_stats();
            }
        }

        stats.m_boxes++;
        env.update_stats();
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
