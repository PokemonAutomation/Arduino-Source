/*  Auto Fossil
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
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
    : NUM_FOSSILS("<b>How Many Fossils to Revive Before Checking Them in Box:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        30, 1, 32*30
    )
    , WHICH_FOSSIL(
        "<b>Which Fossil to Choose in the Dialog Menu:</b>",
        {
            {0, "1st-fossil", "1st Fossil"},
            {1, "2nd-fossil", "2nd Fossil"},
            {2, "3rd-fossil", "3rd Fossil"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , TAKE_VIDEO(
        "Take a video When Found:",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , GO_HOME_WHEN_DONE(true)
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
    PA_ADD_OPTION(NUM_FOSSILS);
    PA_ADD_OPTION(WHICH_FOSSIL);
    PA_ADD_OPTION(STOP_ON);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void AutoFossil::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    const size_t num_fossils_to_revive = NUM_FOSSILS; // at least 1
    const size_t num_boxes = (num_fossils_to_revive+29) / 30;  // at least 1
    while(true){
        for(size_t i = 0; i < num_fossils_to_revive; i++){
            revive_one_fossil(env, context);
            std::ostringstream os;
            os << "Got Fossil " << i + 1 << "/" << num_fossils_to_revive;
            std::string log_str = os.str();
            env.log(log_str);
            env.console.overlay().add_log(log_str);
        }

        overworld_to_box_system(env.console, context);
        for(uint8_t i = 0; i < num_boxes; i++){
            size_t num_fossils_in_box = (i == num_boxes - 1 ? num_fossils_to_revive - i*30 : 30);
            bool found_match = check_fossils_in_one_box(env, context, i*30, num_fossils_in_box);
            if (found_match){
                send_program_finished_notification(env, NOTIFICATION_STATUS);
                return;
            }
            if (i != num_boxes - 1){
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
            10s,
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
            for(size_t i = 0; i < WHICH_FOSSIL.current_value(); i++){
                pbf_press_dpad(context, DPAD_DOWN, 40ms, 40ms);
            }
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
                "revive_one_fossil(): No recognized state after 10 seconds.",
                env.console
            );
        }
    }
}

// start at box system, check fossils one by one
bool AutoFossil::check_fossils_in_one_box(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    size_t num_checked_fossils_in_previous_boxes, size_t num_fossils_in_box)
{
    AutoFossil_Descriptor::Stats& stats = env.current_stats<AutoFossil_Descriptor::Stats>();

    uint8_t box_row = 1, box_col = 0;
    bool next_cell_right = true;
    BoxDetector box_detector(COLOR_RED, &env.console.overlay());
    BoxPageInfoWatcher info_watcher(&env.console.overlay());
    for(size_t i = 0; i < num_fossils_in_box; i++){
        env.console.overlay().add_log("To cell: (" + std::to_string(box_row) + ", " + std::to_string(box_col) + ")");
        box_detector.move_cursor(env.program_info(), env.console, context, box_row, box_col);

        info_watcher.reset_state();
        const int ret = wait_until(env.console, context, std::chrono::seconds(10), {info_watcher});
        if (ret < 0) {
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to detect box info at cell idx " + std::to_string(i) + " after 30 seconds",
                env.console
            );
        }

        
        std::ostringstream os;
        os << num_checked_fossils_in_previous_boxes + i + 1 << "/" << NUM_FOSSILS << ": " << info_watcher.info_str();
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

            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
                context.wait_for_all_requests();
            }
            GO_HOME_WHEN_DONE.run_end_of_program(context);
            return true;
        }

        
        if (next_cell_right){
            if (box_col == 5){
                box_row++;
                size_t remaining_fossils = num_fossils_in_box - 1 - i;
                if (remaining_fossils < 6){
                    // we don't have enough fossils in this last row, so we move
                    // from left to right
                    box_col = 0;
                    next_cell_right = true;
                } else{
                    next_cell_right = false;
                }
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
