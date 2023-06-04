/*  Size Checker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
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
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Boxes Checked");
        m_display_order.emplace_back("Checked");
        m_display_order.emplace_back("Empty Slots");
        m_display_order.emplace_back("Eggs");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& m_boxes;
    std::atomic<uint64_t>& m_checked;
    std::atomic<uint64_t>& m_empty;
    std::atomic<uint64_t>& m_eggs;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> SizeChecker_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



SizeChecker::SizeChecker()
    : GO_HOME_WHEN_DONE(false)
    , BOXES_TO_CHECK(
        "<b>Number of Boxes to Check:</b>",
        LockWhileRunning::UNLOCKED,
        2, 0, 32
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







void SizeChecker::size_check_one(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    SizeChecker_Descriptor::Stats& stats = env.current_stats<SizeChecker_Descriptor::Stats>();

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
        pbf_press_button(context, BUTTON_A, 60, 100);
        pbf_press_button(context, BUTTON_B, 60, 100);
        stats.m_checked++;
        stats.m_errors += errors;
    }catch (OperationFailedException&){
        stats.m_errors++;
        throw;
    }
}
void SizeChecker::size_check_box(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    for (uint8_t row = 0; row < 5; row++){
        for (uint8_t col = 0; col < 6; col++){
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, row, col);
            size_check_one(box_detector, env, context);
            env.update_stats();
        }
    }
}



void SizeChecker::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    SizeChecker_Descriptor::Stats& stats = env.current_stats<SizeChecker_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    BoxDetector box_detector;
    VideoOverlaySet overlays(env.console.overlay());
    box_detector.make_overlays(overlays);

    for (uint8_t box = 0; box < BOXES_TO_CHECK; box++){
        if (box > 0){
            move_to_right_box(context);
        }
        context.wait_for_all_requests();

        env.update_stats();

        size_check_box(box_detector, env, context);

        stats.m_boxes++;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
