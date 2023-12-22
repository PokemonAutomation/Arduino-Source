/*  Flying Trail Farmer
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
#include "PokemonSV_FlyingTrailFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


FlyingTrailFarmer_Descriptor::FlyingTrailFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:FlyingTrailFarmer",
        STRING_POKEMON + " SV", "Flying Trail Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/FlyingTrailFarmer.md",
        "Farm the flying trail for BP.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct FlyingTrailFarmer_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> FlyingTrailFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



FlyingTrailFarmer::FlyingTrailFarmer()
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



void FlyingTrailFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    FlyingTrailFarmer_Descriptor::Stats& stats = env.current_stats<FlyingTrailFarmer_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    // Loop through boxes.
    for (uint8_t box = 0; box < BOXES_TO_CHECK; box++){
        enter_check_mode(env.console, context);
        context.wait_for_all_requests();

        if (box > 0){
            move_to_right_box(context);
        }
        context.wait_for_all_requests();

        // Loop through the rows and columns.
        for (uint8_t row = 0; row < 5; row++){
            for (uint8_t col = 0; col < 6; col++){
                enter_check_mode(env.console, context);
                context.wait_for_all_requests();
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

                exit_check_mode(env.console, context);
                context.wait_for_all_requests();

                stats.m_checked++;
                env.update_stats();
            }
        }

        stats.m_boxes++;
        env.update_stats();
    }

    pbf_press_button(context, BUTTON_B, 20, 20);
    exit_check_mode(env.console, context);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}