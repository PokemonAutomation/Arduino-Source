/*  Auto Fossil
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_MenuNavigation.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxInfoDetector.h"
#include "PokemonLZA_AutoFossil.h"

#include <sstream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;

// event sequence:
// Button "A" on top of researcher Reg when in overworld 
// Flat white dialog box(?) with name Reg x 3
// Flat white dialog box(?) with name Reg + fossil selection menu, green right arrow
// Flat white dialog box(?) with name Reg x 2
// Flash of white screen
// Flat white dialog box (?) with name Reg x 1
// Blue dialog of "You received <fossil pokemon>". This dialog can be cleared by button B as well.
// Flat white dialog box (?) with name Reg x 2
// Back to overworld, A button shown


AutoFossil_Descriptor::AutoFossil_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:AutoFossil",
        STRING_POKEMON + " LZA", "Auto Fossil",
        "Programs/PokemonLZA/AutoFossil.html",
        "Automatically revive fossils.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

class AutoFossil_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : fossils(m_stats["Fossils"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Fossils");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& fossils;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> AutoFossil_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


AutoFossil::AutoFossil(){}


void AutoFossil::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    check_fossils_in_box(env, context, 1);
    return;

    // AutoFossil_Descriptor::Stats& stats = env.current_stats<AutoFossil_Descriptor::Stats>();
    overworld_to_box_system(env.console, context);
    return;

    // Example loop structure
    size_t num_fossils_to_revive = 3;
    for(size_t i = 0; i < num_fossils_to_revive; i++){
        revive_one_fossil(env, context);
        std::ostringstream os;
        os << "Got Fossil " << i + 1 << "/" << num_fossils_to_revive;
        std::string log_str = os.str();
        env.log(log_str);
        env.console.overlay().add_log(log_str);
    }
}

void AutoFossil::revive_one_fossil(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AutoFossil_Descriptor::Stats& stats = env.current_stats<AutoFossil_Descriptor::Stats>();
    bool seen_selection_arrow = false;
    while (true){
        context.wait_for_all_requests();

        ButtonWatcher buttonA_watcher(
            COLOR_WHITE,
            ButtonType::ButtonA,
            {0.1, 0.1, 0.8, 0.8},
            &env.console.overlay()
        );
        SelectionArrowWatcher selection_arrow_watcher(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.6300, 0.4440, 0.2260, 0.3190}
        );

        FlatWhiteDialogWatcher white_dialog_watcher(COLOR_WHITE, &env.console.overlay());
        BlueDialogWatcher blue_dialog_watcher(COLOR_BLUE, &env.console.overlay());
        
        int ret = wait_until(
            env.console, context,
            10000ms,
            {
                buttonA_watcher,
                selection_arrow_watcher,
                white_dialog_watcher,
                blue_dialog_watcher,
            }
        );
        context.wait_for(100ms);

        switch (ret){
        case 0:
            env.log("Detected A button.");
            if (seen_selection_arrow){
                // we finish one round of fossil reviving
                stats.fossils++;
                env.update_stats();
                return;
            }
            // press A to start dialog with researcher Reg to start reviving a new fossil
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 1:
            env.log("Detected selection arrow.");
            // This is when the Reg asks you which fossil to revive
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            seen_selection_arrow = true;
            continue;
        case 2:
        {
            env.log("Detected white dialog.");
            auto button = seen_selection_arrow ? BUTTON_B : BUTTON_A;
            pbf_press_button(context, button, 80ms, 40ms);
            continue;
        }
        case 3:
            env.log("Detected blue dialog.");
            pbf_press_button(context, BUTTON_B, 80ms, 40ms);
            continue;

        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_lobby(): No recognized state after 60 seconds.",
                env.console
            );
        }
    }
}

// start at box system, check fossils one by one
void AutoFossil::check_fossils_in_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t num_boxes){
    uint8_t box_row = 1, box_col = 0;
    bool next_cell_right = true;
    BoxDetector box_detector(COLOR_RED, &env.console.overlay());
    BoxPageInfoWatcher info_watcher(&env.console.overlay());
    for(size_t i = 0; i < 30; i++){
        info_watcher.reset_state();
        wait_until(env.console, context, WallClock::max(), {info_watcher});
        
        std::ostringstream os;
        os << i + 1 << "/" << (num_boxes*30) << ": " << info_watcher.info_str();
        std::string log_str = os.str();
        env.log(log_str);
        env.console.overlay().add_log(log_str);

        if (next_cell_right){
            if (box_col == 5){
                box_row++;
                next_cell_right = false;
            } else{
                box_col++;
            }
        } else{
            if (box_col == 0){
                box_row++;
                next_cell_right = true;
            } else{
                box_col--;
            }
        }
        if (i != 29){
            env.console.overlay().add_log("Next cell: (" + std::to_string(box_row) + ", " + std::to_string(box_col) + ")");
            box_detector.move_cursor(env.program_info(), env.console, context, box_row, box_col);
        }
    }
}

}
}
}
