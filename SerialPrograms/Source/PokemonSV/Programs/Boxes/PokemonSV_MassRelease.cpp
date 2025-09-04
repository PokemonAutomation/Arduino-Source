/*  Mass Release
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
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRelease.h"
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
        "Mass release boxes of " + STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct MassRelease_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_boxes(m_stats["Boxes Cleared"])
        , m_released(m_stats["Released"])
        , m_empty(m_stats["Empty Slots"])
        , m_shinies(m_stats["Shinies"])
        , m_eggs(m_stats["Eggs"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Boxes Cleared");
        m_display_order.emplace_back("Released");
        m_display_order.emplace_back("Empty Slots");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Eggs");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_boxes;
    std::atomic<uint64_t>& m_released;
    std::atomic<uint64_t>& m_empty;
    std::atomic<uint64_t>& m_shinies;
    std::atomic<uint64_t>& m_eggs;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> MassRelease_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



MassRelease::MassRelease()
    : GO_HOME_WHEN_DONE(false)
    , BOXES_TO_RELEASE(
        "<b>Number of Boxes to Release:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        2, 0, 32
    )
    , SKIP_SHINIES(
        "<b>Skip Shinies:</b><br>Do not release shiny " + STRING_POKEMON + ".",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(BOXES_TO_RELEASE);
    PA_ADD_OPTION(SKIP_SHINIES);
    PA_ADD_OPTION(NOTIFICATIONS);
}







void MassRelease::release_one(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    MassRelease_Descriptor::Stats& stats = env.current_stats<MassRelease_Descriptor::Stats>();

    env.log("Selecting " + STRING_POKEMON + "...");

    SomethingInBoxSlotDetector sth_in_box_detector(COLOR_RED);
    BoxCurrentEggDetector egg_detector;
    BoxShinyDetector shiny_detector;
    VideoOverlaySet overlays(env.console.overlay());
    sth_in_box_detector.make_overlays(overlays);
    egg_detector.make_overlays(overlays);
    shiny_detector.make_overlays(overlays);
    context.wait_for_all_requests();

    VideoSnapshot screen = env.console.video().snapshot();

    if (!sth_in_box_detector.detect(screen)){
        stats.m_empty++;
        return;
    }

    // Try to change to stats or judge view
    if (m_in_judge_view == false){
        const bool throw_exception = false;
        if (change_view_to_stats_or_judge(env.console, context, throw_exception)){
            m_in_judge_view = true;
        }else{
            // it is an egg
            stats.m_eggs++;
            return;
        }
        screen = env.console.video().snapshot();
    }

    if (egg_detector.detect(screen)){
        stats.m_eggs++;
        return;
    }

    if (shiny_detector.detect(screen)){
        stats.m_shinies++;
        if (SKIP_SHINIES){
            return;
        }
    }

    try{
        size_t errors = 0;
        release_one_pokemon(env.program_info(), env.console, context, errors);
        stats.m_released++;
        stats.m_errors += errors;
    }catch (OperationFailedException&){
        stats.m_errors++;
        throw;
    }
}
void MassRelease::release_box(BoxDetector& box_detector, SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    for (uint8_t row = 0; row < 5; row++){
        for (uint8_t j_col = 0; j_col < 6; j_col++){
            // Go through slots in a Z-shape pattern
            uint8_t col = (row % 2 == 0 ? j_col : 5 - j_col);
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, row, col);
            release_one(box_detector, env, context);
            env.update_stats();
        }
    }
}



void MassRelease::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    MassRelease_Descriptor::Stats& stats = env.current_stats<MassRelease_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    m_in_judge_view = false;

    BoxDetector box_detector;
    VideoOverlaySet overlays(env.console.overlay());
    box_detector.make_overlays(overlays);

    for (uint8_t box = 0; box < BOXES_TO_RELEASE; box++){
        if (box > 0){
            move_to_right_box(context);
        }
        context.wait_for_all_requests();

        env.update_stats();

        release_box(box_detector, env, context);

        stats.m_boxes++;
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
