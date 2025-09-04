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
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
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
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
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

void ThreeSegmentDudunsparceFinder::check_one_column(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    uint8_t column_index
){
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
    const int empty_party_slots = check_empty_slots_in_party(env.program_info(), env.console, context);
    if (empty_party_slots == 5){
        env.log("Empty column");
        env.console.overlay().add_log("Empty column");
        return;
    }
    const int num_pokemon_in_party = 6 - empty_party_slots;
    const int menu_index = -1;
    // go to bag from box system
    enter_menu_from_box_system(env.program_info(), env.console, context, menu_index);
    enter_bag_from_menu(env.program_info(), env.console, context);
    // move cursor to the "Other Items" tab
    for (size_t c = 0; c < 4; c++){
        pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
    }
    context.wait_for_all_requests();
    // move to candies:

    // Exp M: 3k exp points. from Tera Raid Battles (2★, 3★, 4★)
    // Exp L: 10k exp points. from Tera Raid Battles (4★, 5★, 6★, 7★)
    // Exp XL: 30k exp points. from Tera Raid Battles (5★, 6★, 7★)

    // - 3 exp L to go to lv 31 from lv 1
    // - 4 Exp L to go to lv 34 from lv 1
    // - 11 exp M to go to lv 32 from lv 1
    // - 1 exp M to go from lv 32 to lv 33

    // so total 12 exp M per dunsparce. 12 * 30 = 360 exp M
    // or 3 expl L + 2 exp M per dunsparce. 90 exp M and 60 exp M
    // or 1 exp XL + 2 exp M per dunsparce. 30 exp XL and 60 exp M
    
    // go down 5 to go to candy XL
    for (size_t c = 0; c < 5; c++){
        pbf_press_dpad(context, DPAD_DOWN, 20, 105);
    }
    // press A to bring up sub menu: "Use this item", "Give to Pokemon", "Cancel"
    pbf_press_button(context, BUTTON_A, 20, 105);
    // press A again to select "Use this item"
    pbf_press_button(context, BUTTON_A, 20, 105);
    // now the cursor is on the first pokemon
    const int starting_pokemon_in_party = HAS_CLONE_RIDE_POKEMON ? 2 : 1;
    for (int i_pokemon = starting_pokemon_in_party; i_pokemon < num_pokemon_in_party; i_pokemon++){
        // go down to the target pokemon
        for (int c = 0; c < i_pokemon; c++){
            pbf_press_dpad(context, DPAD_DOWN, 20, 105);
        }
        // select the pokemon. This will open up the item count "x1"
        pbf_press_button(context, BUTTON_A, 20, 105);
        // press A again to apply the x1 candy XL to the pokemon
        pbf_press_button(context, BUTTON_A, 20, 105);
        // wait for some more time to let the level up animation finish
        pbf_wait(context, Seconds(1));
        // press A to clear the "dudunsparce grew up to lv 31" message box
        pbf_press_button(context, BUTTON_A, 20, 105);
    }

    // leave bag and move to menu, enter the first dunsparce's sub-menu
    enter_menu_from_bag(env.program_info(), env.console, context, starting_pokemon_in_party, MenuSide::LEFT);
    // press A again to go into pokemon status summary screen
    pbf_press_button(context, BUTTON_A, 20, 105);
    // wait until we are in pokemon status summary screen
    PokemonSummaryWatcher summary_watcher;
    int ret = wait_until(
        env.console, context,
        Seconds(3),
        {summary_watcher}
    );
    if (ret < 0){
        throw_and_log<FatalProgramException>(
            env.console, ErrorReport::NO_ERROR_REPORT,
            "ThreeSegmentDudunsparceFinder::check_one_column(): No pokemon status summary screen found.",
            env.console
        );
    }
    // move to the moves screen
    pbf_press_dpad(context, DPAD_RIGHT, 20, 105);

    for (int i_pokemon = starting_pokemon_in_party; i_pokemon < num_pokemon_in_party; i_pokemon++){
        // press A to open submenu "Remember moves", "Forget a move", "Use TMs to learn moves", "Quit"
        pbf_press_button(context, BUTTON_A, 20, 105);
        // press A to select "Remember moves"
        pbf_press_button(context, BUTTON_A, 20, 105);
        // wait until Remember Move list to appear
        // XXX
    }
}

void ThreeSegmentDudunsparceFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    ThreeSegmentDudunsparceFinder_Descriptor::Stats& stats = env.current_stats<ThreeSegmentDudunsparceFinder_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 100);

    try{
        for(uint8_t i = 0; i < 6; i++){
            check_one_column(env, context, i);
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
