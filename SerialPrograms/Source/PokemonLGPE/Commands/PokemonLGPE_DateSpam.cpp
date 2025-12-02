/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "Common/Cpp/Time.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "Controllers/SerialPABotBase/Connection/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "PokemonLGPE_DateSpam.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

void verify_date_time_menu_selected(ConsoleHandle& console, JoyconContext& context){
    // Verify that "Date and Time" menu item is selected (not "Time Zone")
    // After home_to_date_time(..., true), we should be in the "Date and Time" menu screen
    // Menu items: "Synchronize Clock via Internet" (top), "Time Zone" (middle), "Date and Time" (bottom)
    // We need "Date and Time" (bottom) to be selected before rolling the date
    
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(300));
    
    // Use single snapshot check instead of polling to avoid visual pulsing
    VideoSnapshot snapshot = console.video().snapshot();
    if (!snapshot){
        console.log("WARNING: No video available for menu verification. Proceeding anyway...", COLOR_RED);
        return;
    }
    
    // Box positions based on Switch menu layout - "Date and Time" text is at bottom
    ImageFloatBox date_time_box(0.50, 0.78, 0.15, 0.03);  // "Date and Time" menu item (bottom)
    ImageFloatBox time_zone_box(0.50, 0.70, 0.15, 0.03);  // "Time Zone" menu item (middle)
    ImageFloatBox sync_clock_box(0.50, 0.62, 0.15, 0.03);  // "Synchronize Clock via Internet" (top)
    
    // Check once if "Date and Time" is selected (using snapshot timestamp, no polling)
    SelectedSettingWatcher date_time_watcher(date_time_box, time_zone_box, sync_clock_box);
    bool date_time_selected = date_time_watcher.process_frame(*snapshot, snapshot.timestamp());
    
    if (date_time_selected){
        // Successfully detected "Date and Time" is selected
        console.log("Verified 'Date and Time' menu item is selected.");
        return;
    }
    
    // "Date and Time" is not selected - navigate to it
    console.log("'Date and Time' not selected. Navigating to it...", COLOR_YELLOW);
    
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 80ms + tv;
    
    // Check if "Time Zone" is selected (middle option)
    SelectedSettingWatcher time_zone_watcher(time_zone_box, date_time_box, sync_clock_box);
    bool time_zone_selected = time_zone_watcher.process_frame(*snapshot, snapshot.timestamp());
    
    if (time_zone_selected){
        // "Time Zone" is selected, need to go down one more to reach "Date and Time"
        console.log("'Time Zone' selected. Moving down one to 'Date and Time'...", COLOR_YELLOW);
        pbf_move_joystick(context, 128, 255, unit, unit);
        context.wait_for_all_requests();
        context.wait_for(Milliseconds(300));
    } else {
        // Might be on "Synchronize Clock via Internet" (top) - need to go down twice
        console.log("Top option selected. Moving down twice to 'Date and Time'...", COLOR_YELLOW);
        pbf_move_joystick(context, 128, 255, unit, unit);
        context.wait_for_all_requests();
        context.wait_for(Milliseconds(200));
        pbf_move_joystick(context, 128, 255, unit, unit);
        context.wait_for_all_requests();
        context.wait_for(Milliseconds(300));
    }
    
    // Final quick check after navigation (single snapshot, no polling)
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(200));
    snapshot = console.video().snapshot();
    if (snapshot){
        date_time_selected = date_time_watcher.process_frame(*snapshot, snapshot.timestamp());
        if (!date_time_selected){
            console.log("WARNING: Could not verify 'Date and Time' is selected after navigation. Proceeding anyway...", COLOR_RED);
        } else {
            console.log("Successfully navigated to 'Date and Time' menu item.");
        }
    }
}

void roll_date_forward_1(JoyconContext& context){
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_move_joystick(context, 128, 0, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);

    pbf_move_joystick(context, 255, 128, 2*unit, unit);
    pbf_move_joystick(context, 128, 0, 2*unit, unit);
    pbf_move_joystick(context, 255, 128, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_move_joystick(context, 255, 128, 2*unit, unit);
    pbf_move_joystick(context, 255, 128, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
}

void roll_date_backward_N(JoyconContext& context, uint8_t skips){
    if (skips == 0){
        return;
    }

    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    pbf_press_button(context, BUTTON_A, 2*unit, unit);

    for (uint8_t c = 0; c < skips - 1; c++){
        pbf_move_joystick(context, 128, 255, 2*unit, unit);
    }

    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_move_joystick(context, 255, 128, 2*unit, unit);

    for (uint8_t c = 0; c < skips - 1; c++){
        pbf_move_joystick(context, 128, 255, 2*unit, unit);
    }

    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_move_joystick(context, 255, 128, 2*unit, unit);
    pbf_move_joystick(context, 255, 128, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
}





}

}
}

