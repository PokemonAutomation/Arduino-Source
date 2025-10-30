/*  Auto Fossil
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxInfoDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA/Programs/PokemonLZA_MenuNavigation.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
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
        , alphas(m_stats["Alphas"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Fossils");
        m_display_order.emplace_back("Alphas");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& fossils;
    std::atomic<uint64_t>& alphas;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> AutoFossil_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


AutoFossil::AutoFossil()
    : NUM_BOXES("<b>Boxes of Fossils to Revive:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 32
    )
    , FOUND_SHINY_OR_ALPHA(
        "Found Shiny or Alpha",
        true, true,
        ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &FOUND_SHINY_OR_ALPHA,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STOP_ON);
    PA_ADD_OPTION(NUM_BOXES);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void AutoFossil::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    while(true){
        size_t num_fossils_to_revive = size_t(NUM_BOXES) * 30;
        for(size_t i = 0; i < num_fossils_to_revive; i++){
            revive_one_fossil(env, context);
            std::ostringstream os;
            os << "Got Fossil " << i + 1 << "/" << num_fossils_to_revive;
            std::string log_str = os.str();
            env.log(log_str);
            env.console.overlay().add_log(log_str);
        }

        overworld_to_box_system(env.console, context);
        for(uint8_t i = 0; i < NUM_BOXES; i++){
            bool found_match = check_fossils_in_one_box(env, context);
            if (found_match){
                return;
            }
            if (i != NUM_BOXES - 1){
                // go to next page
                pbf_press_button(context, BUTTON_R, 200ms, 200ms);
            }
        }
        // checked all boxes, no match
        go_home(env.console, context);
        reset_game_from_home(env, env.console, context);
        
        send_program_status_notification(env, NOTIFICATION_STATUS);
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
            // in normal cases when blue dialog box is detected, we already seen the selection
            // arrow. But just in case somehow the program misses the selection arrow detection,
            // we can still set it to true here to ensure subsequent program execution is smooth.
            seen_selection_arrow = true;
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
bool AutoFossil::check_fossils_in_one_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AutoFossil_Descriptor::Stats& stats = env.current_stats<AutoFossil_Descriptor::Stats>();

    uint8_t box_row = 1, box_col = 0;
    bool next_cell_right = true;
    BoxDetector box_detector(COLOR_RED, &env.console.overlay());
    BoxPageInfoWatcher info_watcher(&env.console.overlay());
    for(size_t i = 0; i < 30; i++){
        env.console.overlay().add_log("To cell: (" + std::to_string(box_row) + ", " + std::to_string(box_col) + ")");
        box_detector.move_cursor(env.program_info(), env.console, context, box_row, box_col);

        info_watcher.reset_state();
        wait_until(env.console, context, WallClock::max(), {info_watcher});
        
        std::ostringstream os;
        os << i + 1 << "/" << int(NUM_BOXES)*30 << ": " << info_watcher.info_str();
        std::string log_str = os.str();
        env.log(log_str);
        env.console.overlay().add_log(log_str);
        if (info_watcher.is_alpha()){
            stats.alphas++;
            env.update_stats();
        }
        if (info_watcher.is_shiny()){
            stats.shinies++;
            env.update_stats();
        }

        bool is_match = false;
        switch (STOP_ON){
        case PokemonLA::StopOn::Shiny:
            is_match = info_watcher.is_shiny();
            break;
        case PokemonLA::StopOn::Alpha:
            is_match = info_watcher.is_alpha();
            break;
        case PokemonLA::StopOn::ShinyOrAlpha:
            is_match = info_watcher.is_shiny() || info_watcher.is_alpha();
            break;
        case PokemonLA::StopOn::ShinyAndAlpha:
            is_match = info_watcher.is_shiny() && info_watcher.is_alpha();
            break;
        }
        if (is_match){
            send_program_notification(
                env, FOUND_SHINY_OR_ALPHA,
                Pokemon::COLOR_STAR_SHINY,
                "Found " + info_watcher.info_str() + "!",
                {}, "",
                env.console.video().snapshot(), true
            );
            return true;
        }

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
    }
    return false;
}

}
}
}
