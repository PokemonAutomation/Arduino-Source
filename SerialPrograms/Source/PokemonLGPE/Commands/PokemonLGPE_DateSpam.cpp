/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "Common/Cpp/Time.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
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
    // Comprehensive verification: ensure we're on "Date and Time" menu item AND Sync Clock is OFF
    // Menu structure: "Synchronize Clock via Internet" (top), "Time Zone" (middle), "Date and Time" (bottom)
    // CRITICAL: Must be on "Date and Time" before rolling date, never on "Time Zone" or "Sync Clock"
    
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(400));
    
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 100ms + tv;
    
    // STEP 1: Check and disable "Synchronize Clock via Internet" if it's ON
    console.log("Step 1: Checking 'Synchronize Clock via Internet' status...");
    VideoSnapshot snapshot = console.video().snapshot();
    if (!snapshot){
        console.log("WARNING: No video available. Proceeding blind...", COLOR_RED);
        return;
    }
    
    // Add visual overlay boxes so you can see where we're looking
    VideoOverlaySet overlays(console.overlay());
    
    // Based on your screenshot: menu items are in the lower half of the screen
    // "Synchronize Clock via Internet" is the first item, with "On"/"Off" status on the right
    // "Time Zone" shows "Denver" on the right around y=0.70
    // So "Synchronize Clock" status should be above that, around y=0.55-0.65
    
    ImageFloatBox sync_status_box1(0.78, 0.21, 0.05, 0.05);  // Try y=0.50
    
    overlays.add(COLOR_RED, sync_status_box1);
    
    ImageStats sync_status1 = image_stats(extract_box_reference(snapshot, sync_status_box1));
    
    // Debug: Log all positions
    console.log("Box1 (RED, y:0.21) RGB: [" + std::to_string((int)sync_status1.average.r) + ", " + 
                std::to_string((int)sync_status1.average.g) + ", " + 
                std::to_string((int)sync_status1.average.b) + "] sum=" + 
                std::to_string((int)sync_status1.average.sum()));
    console.log("LOOK AT THE COLORED BOXES ON SCREEN - which one covers the 'On' or 'Off' text?");
    
    // Use the detected values
    ImageStats sync_status = sync_status1;
    
    // "On" is cyan/teal text: green and blue are higher than red
    // "Off" is white text: all RGB components are similar and high
    // The cyan "On" text shows as [43, 56, 53] - green and blue slightly higher than red
    // Detection: if G > R+5 AND B > R, it's likely cyan "On"
    // If all components are similar (within 10 of each other), it's white "Off"
    
    bool is_cyan = (sync_status.average.g > sync_status.average.r + 5) && 
                   (sync_status.average.b >= sync_status.average.r);
    bool is_white = (std::abs(sync_status.average.r - sync_status.average.g) < 10) &&
                    (std::abs(sync_status.average.r - sync_status.average.b) < 10) &&
                    (std::abs(sync_status.average.g - sync_status.average.b) < 10);
    
    bool sync_appears_on = is_cyan && !is_white;
    
    console.log("Is cyan: " + std::string(is_cyan ? "YES" : "NO") + 
                ", Is white: " + std::string(is_white ? "YES" : "NO") +
                ", Appears ON: " + std::string(sync_appears_on ? "YES" : "NO"));
    
    if (sync_appears_on){
        console.log("DETECTED: 'Synchronize Clock via Internet' is ON. Disabling it...", COLOR_YELLOW);
        
        // Strategy: First scroll down to bottom (Date and Time), then scroll up exactly 2 times to Sync Clock
        // This ensures we know exactly where we are
        console.log("Navigating to bottom of menu first...");
        for (int i = 0; i < 5; i++){
            pbf_move_joystick(context, 128, 255, unit, unit);  // Down - go to bottom
            context.wait_for_all_requests();
            context.wait_for(Milliseconds(100));
        }
        
        console.log("Now navigating UP to 'Synchronize Clock via Internet'...");
        // From "Date and Time" (bottom), go up 2 times to reach "Synchronize Clock" (top)
        pbf_move_joystick(context, 128, 0, unit, unit);  // Up to Time Zone
        context.wait_for_all_requests();
        context.wait_for(Milliseconds(250));
        pbf_move_joystick(context, 128, 0, unit, unit);  // Up to Synchronize Clock
        context.wait_for_all_requests();
        context.wait_for(Milliseconds(400));
        
        // Press A to toggle it OFF
        console.log("Pressing A to toggle OFF...");
        pbf_press_button(context, BUTTON_A, unit, unit);
        context.wait_for_all_requests();
        context.wait_for(Milliseconds(600));  // Wait for toggle animation
        
        console.log("Toggled 'Synchronize Clock via Internet' OFF. Now navigating back to 'Date and Time'...");
        
        // Navigate back down to "Date and Time" (2 times down)
        pbf_move_joystick(context, 128, 255, unit, unit);  // Down to Time Zone
        context.wait_for_all_requests();
        context.wait_for(Milliseconds(250));
        pbf_move_joystick(context, 128, 255, unit, unit);  // Down to Date and Time
        context.wait_for_all_requests();
        context.wait_for(Milliseconds(400));
        
        console.log("Back on 'Date and Time' menu item.");
    } else {
        console.log("'Synchronize Clock via Internet' is OFF (or detection failed - check RGB values above).");
    }
    
    // STEP 2: Ensure we're on "Date and Time" menu item (bottom option)
    console.log("Step 2: Ensuring 'Date and Time' menu item is selected...");
    
    // After handling Sync Clock, we should be on "Date and Time" already if we navigated correctly
    // But let's verify and fix if needed
    
    // Simple approach: Just scroll down multiple times to guarantee we're at the bottom
    // The menu will stop at "Date and Time" (bottom item) even if we scroll too much
    console.log("Scrolling to bottom to ensure 'Date and Time' is selected...");
    for (int i = 0; i < 5; i++){
        pbf_move_joystick(context, 128, 255, unit, unit);  // Down
        context.wait_for_all_requests();
        context.wait_for(Milliseconds(150));
    }
    
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(400));
    
    // Final verification with visual boxes to see where menu items actually are
    snapshot = console.video().snapshot();
    if (snapshot){
        // Show boxes at different Y positions to find where menu items are
        // These boxes cover the LEFT side where menu item TEXT appears (not status text)
        ImageFloatBox menu_check_box1(0.15, 0.15, 0.40, 0.05);  // Very top
        ImageFloatBox menu_check_box2(0.15, 0.25, 0.40, 0.05);  // Upper area
        ImageFloatBox menu_check_box3(0.15, 0.35, 0.40, 0.05);  // Middle area
        ImageFloatBox menu_check_box4(0.15, 0.45, 0.40, 0.05);  // Lower-middle area
        ImageFloatBox menu_check_box5(0.15, 0.55, 0.40, 0.05);  // Lower area
        
        VideoOverlaySet menu_overlays(console.overlay());
        menu_overlays.add(COLOR_CYAN, menu_check_box1);
        menu_overlays.add(COLOR_MAGENTA, menu_check_box2);
        menu_overlays.add(COLOR_GREEN, menu_check_box3);
        menu_overlays.add(COLOR_ORANGE, menu_check_box4);
        menu_overlays.add(COLOR_PURPLE, menu_check_box5);
        
        ImageStats menu_stats1 = image_stats(extract_box_reference(snapshot, menu_check_box1));
        ImageStats menu_stats2 = image_stats(extract_box_reference(snapshot, menu_check_box2));
        ImageStats menu_stats3 = image_stats(extract_box_reference(snapshot, menu_check_box3));
        ImageStats menu_stats4 = image_stats(extract_box_reference(snapshot, menu_check_box4));
        ImageStats menu_stats5 = image_stats(extract_box_reference(snapshot, menu_check_box5));
        
        console.log("Menu boxes - CYAN(y:0.15):" + std::to_string((int)menu_stats1.average.sum()) +
                    " MAGENTA(y:0.25):" + std::to_string((int)menu_stats2.average.sum()) +
                    " GREEN(y:0.35):" + std::to_string((int)menu_stats3.average.sum()) +
                    " ORANGE(y:0.45):" + std::to_string((int)menu_stats4.average.sum()) +
                    " PURPLE(y:0.55):" + std::to_string((int)menu_stats5.average.sum()));
        console.log("LOOK: Which colored box covers the HIGHLIGHTED menu item (should have blue border)?");
        console.log("The highlighted item should be 'Date and Time' (bottom option).");
    }
    
    console.log("Ready to roll date. 'Date and Time' should now be selected.");
}

void reset_sync_clock_state(ConsoleHandle& console, JoyconContext& context){
    // Reset the menu state by toggling "Synchronize Clock via Internet" ON then OFF
    // This helps clear any accumulated navigation errors
    
    console.log("=== RESETTING SYNC CLOCK STATE ===", COLOR_BLUE);
    
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 100ms + tv;
    
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(300));
    
    // Navigate to bottom first to establish known position
    console.log("Scrolling to bottom...");
    for (int i = 0; i < 5; i++){
        pbf_move_joystick(context, 128, 255, unit, unit);
        context.wait_for_all_requests();
        context.wait_for(Milliseconds(100));
    }
    
    // Navigate up to "Synchronize Clock via Internet" (top item)
    console.log("Navigating UP to 'Synchronize Clock via Internet'...");
    pbf_move_joystick(context, 128, 0, unit, unit);  // Up to Time Zone
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(200));
    pbf_move_joystick(context, 128, 0, unit, unit);  // Up to Synchronize Clock
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(400));
    
    // Toggle it ON
    console.log("Toggling ON...");
    pbf_press_button(context, BUTTON_A, unit, unit);
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(500));
    
    // Toggle it OFF
    console.log("Toggling OFF...");
    pbf_press_button(context, BUTTON_A, unit, unit);
    context.wait_for_all_requests();
    context.wait_for(Milliseconds(500));
    
    console.log("Sync clock state reset complete.");
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

