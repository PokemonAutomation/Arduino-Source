/*  Three-Segment Dudunsparce Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV_ThreeSegmentDudunsparceFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

ThreeSegmentDudunsparceFinder_Descriptor::ThreeSegmentDudunsparceFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ThreeSegmentDudunsparceFinder",
        STRING_POKEMON + " SV", "Three-Segment Dudunsparce Finder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ThreeSegmentDudunsparceFinder.md",
        "Check whether a box of Dunsparce contain at least one that evolves into Three-Segment Dudunsparce.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_ProController},
        FasterIfTickPrecise::NOT_FASTER
    )
{}
struct ThreeSegmentDudunsparceFinder_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_attempts(m_stats["Attempts"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_attempts;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> ThreeSegmentDudunsparceFinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



ThreeSegmentDudunsparceFinder::ThreeSegmentDudunsparceFinder()
    : GO_HOME_WHEN_DONE(false)
    , HAS_CLONE_RIDE_POKEMON(
        "<b>Cloned Ride Legendary 2nd in Party:</b><br>"
        "Ride legendary cannot be cloned after patch 1.0.1. To preserve the existing clone while hatching eggs, "
        "place it as second in party before starting the program.</b>"
        "The program will skip the first row of eggs in the box as a result.",
        LockMode::LOCK_WHILE_RUNNING,
        false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(HAS_CLONE_RIDE_POKEMON);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ThreeSegmentDudunsparceFinder::check_one_column(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int column_index){
    enter_box_system_from_overworld(env.program_info(), env.console, context);
    const uint8_t expected_empty_slots_in_party = HAS_CLONE_RIDE_POKEMON ? 4 : 5;
    if (check_empty_slots_in_party(env.program_info(), env.console, context) != expected_empty_slots_in_party){
        throw_and_log<FatalProgramException>(
            env.console, ErrorReport::SEND_ERROR_REPORT,
            "Your party should have " + std::to_string(expected_empty_slots_in_party) + " " + STRING_POKEMON + ".",
            env.console
        );
    }

    load_one_column_to_party(env, env.console, context, NOTIFICATION_ERROR_FATAL, column_index, HAS_CLONE_RIDE_POKEMON);
    const int menu_index = 0;
    // go to bag from box system
    enter_menu_from_box_system(env.program_info(), env.console, context, menu_index, MenuSide::RIGHT);
    context.wait_for_all_requests();
    // BagWatcher bag_detector(BagWatcher::FinderType::PRESENT, COLOR_GREEN);
}

void ThreeSegmentDudunsparceFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    ThreeSegmentDudunsparceFinder_Descriptor::Stats& stats = env.current_stats<ThreeSegmentDudunsparceFinder_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 100);

    try{
        for(int i = 0; i < 6; i++){
            check_one_column(env, context, i);
            break; // XXX
        }
    } catch(OperationFailedException&){
        stats.m_errors++;
        env.update_stats();
        throw;
    }

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}






}
}
}
