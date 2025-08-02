/*  PokemonSV World Navigation
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/UnexpectedBattleException.h"
#include "CommonTools/Async/InferenceRoutines.h"
//#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
//#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
//#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
//#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapMenuDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapPokeCenterIconDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_PicnicDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_ZeroGateWarpPromptDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV_ConnectToInternet.h"
#include "PokemonSV_WorldNavigation.h"

#include <array>
#include <cmath>
#include <sstream>
#include <cfloat>
#include <iostream>
namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


bool fly_to_overworld_from_map(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, bool check_fly_menuitem){
    context.wait_for_all_requests();
    // Press A to bring up the promp dialog on choosing "Fly here", "Set as destination", "Never mind".
    pbf_press_button(context, BUTTON_A, 20, 130);

    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(2)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "fly_to_overworld_from_map(): Failed to fly from map after 2 minutes.",
                stream
            );
        }

        int ret = 0;
        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        WhiteButtonWatcher map(COLOR_RED, WhiteButton::ButtonY, {0.800, 0.118, 0.030, 0.060});
        GradientArrowWatcher spot_dialog_watcher(COLOR_YELLOW, GradientArrowType::RIGHT, {0.469, 0.500, 0.215, 0.150});
        PromptDialogWatcher confirm_watcher(COLOR_BLUE, {0.686, 0.494, 0.171, 0.163});
        MapFlyMenuWatcher flyMenuItemWatcher(COLOR_BLACK);
        MapDestinationMenuWatcher destinationMenuItemWatcher(COLOR_BLACK);

        context.wait_for_all_requests();

        std::vector<PeriodicInferenceCallback> callbacks{overworld, map, spot_dialog_watcher, confirm_watcher};
        if (check_fly_menuitem){
            callbacks[2] = flyMenuItemWatcher;
            callbacks.push_back(destinationMenuItemWatcher);
        }

        ret = wait_until(stream, context, std::chrono::minutes(2), callbacks);
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            stream.log("Detected overworld. Fly successful.");
            return true;
        case 1:
            stream.log("Detected map. Pressing A to open map menu.");
            // Press A to bring up the promp dialog on choosing "Fly here", "Set as destination", "Never mind".
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 2:
            stream.log("Detected fly here prompt dialog.");
            stream.overlay().add_log("Fly");
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 3:
            stream.log("Detected fly confirmation prompt.");
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 4:
            stream.log("Detected no fly spot here.");
            stream.overlay().add_log("No fly spot", COLOR_RED);
            return false;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "fly_to_overworld_from_map(): No recognized state after 2 minutes.",
                stream
            );
        }
    }
}


void picnic_from_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    stream.log("Start picnic from overworld...");
    WallClock start = current_time();
    bool success = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(3)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "picnic_from_overworld(): Failed to start picnic after 3 minutes.",
                stream
            );
        }

        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        MainMenuWatcher main_menu(COLOR_RED);
        PicnicWatcher picnic;
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {overworld, main_menu, picnic}
        );
        context.wait_for(std::chrono::milliseconds(100));
        const bool fast_mode = false;
        switch (ret){
        case 0:
            stream.log("Detected overworld.");
            pbf_press_button(context, BUTTON_X, 20, 105); // open menu 
            continue;
        case 1:
            stream.log("Detected main menu.");
            success = main_menu.move_cursor(info, stream, context, MenuSide::RIGHT, 2, fast_mode);
            if (success == false){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "picnic_from_overworld(): Cannot move menu cursor to picnic.",
                    stream
                );
            }
            pbf_mash_button(context, BUTTON_A, 125); // mash button A to enter picnic mode
            continue;
        case 2:
            stream.log("Detected picnic.");
            stream.overlay().add_log("Start picnic", COLOR_WHITE);
            // extra wait to make sure by the end the player can move.
            // the player throwing out pokeballs animation is long.
            pbf_wait(context, 1000);
            context.wait_for_all_requests();
            return;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "picnic_from_overworld(): No recognized state after 30 seconds.",
                stream
            );
        }
    }
}

void leave_picnic(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    stream.log("Leaving picnic...");
    stream.overlay().add_log("Leaving picnic", COLOR_WHITE);

    pbf_press_button(context, BUTTON_Y, 30, 100);
    for(int i = 0; i < 5; i++){
        PromptDialogWatcher prompt(COLOR_RED, {0.595, 0.517, 0.273, 0.131});
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(5),
            {prompt}
        );

        if (ret == 0){
            stream.log("Detected end picnic prompt");
            break;
        }

        if (i == 4){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "leave_picnic(): Failed to leave picnic after 5 tries.",
                stream
            );
        }

        // prompt not found, maybe button Y dropped?
        pbf_press_button(context, BUTTON_Y, 30, 100);
    }

    // We have now the prompt to asking for confirmation of leaving picnic.
    // Mash A to confirm
    pbf_mash_button(context, BUTTON_A, 150);
    context.wait_for_all_requests();
    
    // Wait for overworld:
    OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
    int ret = wait_until(
        stream, context,
        std::chrono::seconds(20),
        {overworld}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "leave_picnic(): Failed to detect overworld after 20 seconds.",
            stream
        );
    }
    // Wait three more seconds to make sure the player character is free to operate:
    context.wait_for(std::chrono::seconds(3));
}


// While in the current map zoom level, detect pokecenter icons and move the map cursor there.
// Return true if succeed. Return false if no visible pokcenter on map
bool detect_closest_pokecenter_and_move_map_cursor_there(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    double push_scale
){
    context.wait_for_all_requests();
    const auto snapshot_frame = stream.video().snapshot().frame;
    const size_t screen_width = snapshot_frame->width();
    const size_t screen_height = snapshot_frame->height();

    double closest_icon_x = 0., closest_icon_y = 0.;
    double max_dist = DBL_MAX;
    const double center_x = 0.5 * screen_width, center_y = 0.5 * screen_height;
    {
        MapPokeCenterIconWatcher pokecenter_watcher(COLOR_RED, stream.overlay(), MAP_READABLE_AREA);
        int ret = wait_until(stream, context, std::chrono::seconds(2), {pokecenter_watcher});
        if (ret != 0){
            stream.log("No visible pokecetner found on map");
            stream.overlay().add_log("No whole PokeCenter icon");
            return false;
        }
        // Find the detected PokeCenter icon closest to the screen center (where player character is on the map).
        for(const auto& box: pokecenter_watcher.found_locations()){
            const double loc_x = (box.x + box.width/2) * screen_width;
            const double loc_y = (box.y + box.height/2) * screen_height;
            const double x_diff = loc_x - center_x, y_diff = loc_y - center_y;
            const double dist2 = x_diff * x_diff + y_diff * y_diff;
            std::ostringstream os;
            os << "Found pokecenter at box: x=" << box.x << ", y=" << box.y << ", width=" << box.width << ", height=" << box.height << 
                ", dist to center " << std::sqrt(dist2) << " pixels";
            stream.log(os.str());

            if (dist2 < max_dist){
                max_dist = dist2;
                closest_icon_x = loc_x; closest_icon_y = loc_y;
            }
        }
        stream.log("Found closest pokecenter icon on map: (" + std::to_string(closest_icon_x) + ", " + std::to_string(closest_icon_y) + ").");
        stream.overlay().add_log("Detected PokeCenter icon");
    }

    // Convert the vector from center to the PokeCenter icon into a left joystick movement
    const double dif_x = (closest_icon_x - center_x) * 1920/ screen_width;
    const double dif_y = (closest_icon_y - center_y) * 1080/ screen_height;
    const double magnitude = std::max(std::sqrt(max_dist), 1.0);
    const double push_x = dif_x * 64 / magnitude, push_y = dif_y * 64 / magnitude;

    // 0.5 is too large, 0.25 a little too small, 0.30 is a bit too much for a far-away pokecenter
    const double scale = push_scale;

    const uint8_t move_x = uint8_t(std::max(std::min(int(round(push_x + 128) + 0.5), 255), 0));
    const uint8_t move_y = uint8_t(std::max(std::min(int(round(push_y + 128) + 0.5), 255), 0));

    stream.overlay().add_log("Move Cursor to PokeCenter", COLOR_WHITE);
    const uint16_t push_time = std::max(uint16_t(magnitude * scale + 0.5), uint16_t(3));
    pbf_move_left_joystick(context, move_x, move_y, push_time, 30);
    context.wait_for_all_requests();
    return true;
}

// While in the current map zoom level, detect pokecenter icons and fly to the closest one.
// Return true if succeed. Return false if no visible pokcenter on map
// Throw Operation failed Exception if detected pokecenter, but failed to fly there.
bool fly_to_visible_closest_pokecenter_cur_zoom_level(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    double push_scale
){
    if (!detect_closest_pokecenter_and_move_map_cursor_there(info, stream, context, push_scale)){
        return false;
    }
    bool check_fly_menuitem = true;
    const bool success = fly_to_overworld_from_map(info, stream, context, check_fly_menuitem);
    if (success){
        return true;
    }else{
        // detected pokecenter, but failed to fly there.
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "fly_to_visible_closest_pokecenter_cur_zoom_level(): Detected pokecenter, but failed to fly there as no \"Fly\" menuitem.",
            stream
        );
    }

}


void fly_to_closest_pokecenter_on_map(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    const int MAX_TRY_COUNT = 17;
    int try_count = 0;
    // Part 1: Tries to detect a pokecenter that is very close to the player
    // Zoom in one level onto the map.
    // If the player character icon or any wild pokemon icon overlaps with the PokeCenter icon, the code cannot
    // detect it. So we zoom in as much as we can to prevent any icon overlap.

    // failures to fly to pokecenter are often when the Switch lags. from my testing, a 1.4-1.5 adjustment factor seems to work
    const std::array<double, MAX_TRY_COUNT> adjustment_table =  {1, 1.4, 1, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 0.9, 0.8, 1.4}; // {1, 1.4, 1.5};
    
    while(true){
        try {
            pbf_press_button(context, BUTTON_ZR, 40, 100);
            // try different magnitudes of cursor push with each failure.
            double push_scale = 0.29 * adjustment_table[try_count];
            // std::cout << "push_scale: " << std::to_string(push_scale) << std::endl;
            if (fly_to_visible_closest_pokecenter_cur_zoom_level(info, stream, context, push_scale)){
                return; // success in finding the closest pokecenter. Return.
            }

            // no visible pokecenters at this zoom level. Move on to part 2.
            break;
        }catch (OperationFailedException&){ // pokecenter was detected, but failed to fly there
            try_count++;
            if (try_count >= MAX_TRY_COUNT){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "fly_to_closest_pokecenter_on_map(): At min warpable map level, pokecenter was detected, but failed to fly there.",
                    stream
                );                
            }
            stream.log("Failed to find the fly menu item. Restart the closest Pokecenter travel process.");
            press_Bs_to_back_to_overworld(info, stream, context);
            open_map_from_overworld(info, stream, context);
        }
    }

    // Part 2: Tries to detect any pokecenter that is overlapped with the player.
    // Zoom out to the max warpable level and try pressing on the player character.
    stream.log("Zoom to max map level to try searching for Pokecenter again.");
    stream.overlay().add_log("Pokecenter Icon occluded");
    pbf_press_button(context, BUTTON_ZL, 40, 100);
    pbf_press_button(context, BUTTON_ZL, 40, 100);

    const bool check_fly_menuitem = true;
    if (fly_to_overworld_from_map(info, stream, context, check_fly_menuitem)){
        return; // success in flying to the pokecenter that overlaps with the player character at max warpable level.
    }

    // Failed to find pokecenter overlapping with player
    stream.log("No PokeCenter icon overlapping with the player character on the max warpable level");
    stream.overlay().add_log("No overlapping PokeCenter");
    // press B to close the destination menu item
    pbf_press_button(context, BUTTON_B, 60, 100);


    // Part 3: Tries to detect a pokecenter that is further away from the player, while at max warpable level
    try_count = 0;
    while(true){
        try {
            double push_scale = 0.29 * adjustment_table[try_count];
            // std::cout << "push_scale: " << std::to_string(push_scale) << std::endl;
            // Now try finding the closest pokecenter at the max warpable level
            if (fly_to_visible_closest_pokecenter_cur_zoom_level(info, stream, context, push_scale)){
                return; // success in finding the closest pokecenter. Return.
            }else{
                // Does not detect any pokecenter on map
                stream.overlay().add_log("Still no PokeCenter Found!", COLOR_RED);
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "fly_to_closest_pokecenter_on_map(): At max warpable map level, still cannot find PokeCenter icon.",
                    stream
                );
            }
        }catch (OperationFailedException& e){ 
            try_count++;
            if (try_count >= MAX_TRY_COUNT){
                // either: 
                // - pokecenter was detected, but failed to fly there. 
                // - could not find pokecenter icon.
                throw e;
            }
            stream.log("Failed to find the fly menuitem. Restart the closest Pokecenter travel process.");
            press_Bs_to_back_to_overworld(info, stream, context);
            open_map_from_overworld(info, stream, context);
            // zoom out to max warpable level
            pbf_press_button(context, BUTTON_ZL, 40, 100);
        }
    }
}

void jump_off_wall_until_map_open(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    for (auto i = 0; i < 3; i++){
        pbf_press_button(context, BUTTON_L, 50, 50);
        pbf_press_button(context, BUTTON_B, 50, 50);
        pbf_move_left_joystick(context, 128, 255, 100, 50);
        context.wait_for_all_requests();
        try{
            open_map_from_overworld(info, stream, context);
            break;
        }
        catch(...){
            stream.log("Failed to open map.");
        }
        if (i >= 3){
            stream.log("Could not escape wall.");
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "jump_off_wall_until_map_open(): Could not escape wall.",
                stream
            );
        }
    }
}

// Open map and teleport back to town pokecenter
void reset_to_pokecenter(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    while (true){
        try {
            open_map_from_overworld(info, stream, context);
            fly_to_closest_pokecenter_on_map(info, stream, context);
            break;
        }catch (UnexpectedBattleException&){
            run_wild_battle_press_A(stream, context, BattleStopCondition::STOP_OVERWORLD);
        }
    }

}

void realign_player(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    PlayerRealignMode realign_mode,
    uint8_t move_x, uint8_t move_y, uint16_t move_duration
){
    stream.log("Realigning player direction...");
    switch (realign_mode){
    case PlayerRealignMode::REALIGN_NEW_MARKER:
        stream.log("Setting new map marker...");
        open_map_from_overworld(info, stream, context);
        pbf_press_button(context, BUTTON_ZR, 20, 105);
        pbf_move_left_joystick(context, move_x, move_y, move_duration, 1 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        leave_phone_to_overworld(info, stream, context);
        return;     
    case PlayerRealignMode::REALIGN_OLD_MARKER:
        open_map_from_overworld(info, stream, context, false);
        leave_phone_to_overworld(info, stream, context);
        pbf_press_button(context, BUTTON_L, 20, 105);
        return;
    case PlayerRealignMode::REALIGN_NO_MARKER:
        pbf_move_left_joystick(context, move_x, move_y, move_duration, 1 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_L, 20, 105);
        return;
    }  

}


void walk_forward_until_dialog(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    NavigationMovementMode movement_mode,
    uint16_t seconds_timeout,
    uint8_t x,
    uint8_t y
){

    DialogBoxWatcher        dialog(COLOR_RED, true);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            ssf_press_left_joystick(context, 128, y, 0, seconds_timeout * TICKS_PER_SECOND);
            if (movement_mode == NavigationMovementMode::DIRECTIONAL_ONLY){
                pbf_wait(context, seconds_timeout * TICKS_PER_SECOND);
            } else if (movement_mode == NavigationMovementMode::DIRECTIONAL_SPAM_A){
                pbf_mash_button(context, BUTTON_A, seconds_timeout * TICKS_PER_SECOND);
                // for (size_t j = 0; j < seconds_timeout; j++){
                //     pbf_press_button(context, BUTTON_A, 20, 105);
                // }
            }
        },
        {dialog}
    );
    context.wait_for(std::chrono::milliseconds(100));

    switch (ret){
    case 0: // dialog
        stream.log("walk_forward_until_dialog(): Detected dialog.");
        return;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "walk_forward_until_dialog(): Timed out. Did not detect dialog.",
            stream
        );
    }
}

void walk_forward_while_clear_front_path(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    uint16_t forward_ticks,
    uint8_t y,
    uint16_t ticks_between_lets_go,
    uint16_t delay_after_lets_go
){
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_R, 20, delay_after_lets_go);

    uint16_t num_ticks_left = forward_ticks;
    while (true){

        if (num_ticks_left < ticks_between_lets_go){
            pbf_move_left_joystick(context, 128, y, num_ticks_left, 20);
            context.wait_for_all_requests();
            stream.log("walk_forward_while_clear_front_path() ticks traveled: " + std::to_string(forward_ticks));
            break;
        }

        pbf_move_left_joystick(context, 128, y, ticks_between_lets_go, 20);
        num_ticks_left -= ticks_between_lets_go;

        context.wait_for_all_requests();
        stream.log("walk_forward_while_clear_front_path() ticks traveled: " + std::to_string(forward_ticks - num_ticks_left));

        pbf_press_button(context, BUTTON_R, 20, delay_after_lets_go);
        

    }
}


bool attempt_fly_to_overlapping_flypoint(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context
){
    while (true){
        try {
            open_map_from_overworld(info, stream, context, false);
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_ZL, 40, 100);

            return fly_to_overworld_from_map(info, stream, context, true);

        }catch (UnexpectedBattleException&){
            run_wild_battle_press_A(stream, context, BattleStopCondition::STOP_OVERWORLD);
        }
    }

}

void fly_to_overlapping_flypoint(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    if (!attempt_fly_to_overlapping_flypoint(info, stream, context)){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to reset to overlapping Pokecenter.",
            stream
        );
    }
}

void confirm_no_overlapping_flypoint(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    if (attempt_fly_to_overlapping_flypoint(info, stream, context)){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Overlapping fly detected, when it wasn't expected.",
            stream
        );
    }
}


void heal_at_pokecenter(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    
    if (!attempt_fly_to_overlapping_flypoint(info, stream, context)){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to fly to pokecenter.",
            stream
        );  
    }           
    uint16_t seconds_timeout = 60;

    // re-orient camera
    pbf_press_button(context, BUTTON_L, 20, 20);
    // move towards pokecenter
    pbf_move_left_joystick(context, 128, 255, 100, 20);
    // re-orient camera
    pbf_press_button(context, BUTTON_L, 20, 20); 

    bool seen_prompt = false;

    while (true){
        OverworldWatcher    overworld(stream.logger(), COLOR_CYAN);
        // TODO: test the Prompt watcher on all languages. Ensure FloatBox is sized correctly.
        PromptDialogWatcher prompt(COLOR_YELLOW, {0.50, 0.400, 0.400, 0.080}); // 0.630, 0.400, 0.100, 0.080 // {0.50, 0.40, 0.40, 0.50}
        AdvanceDialogWatcher    advance_dialog(COLOR_RED);
        TutorialWatcher     tutorial(COLOR_RED);
        context.wait_for_all_requests();

        int ret = wait_until(
            stream, context,
            std::chrono::seconds(seconds_timeout),
            {overworld, prompt, advance_dialog, tutorial}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0: // overworld
            stream.log("heal_at_pokecenter: Detected overworld.");
            if (seen_prompt){ 
                // if have seen the prompt dialog and are now in overworld, assume we have healed
                stream.log("heal_at_pokecenter: Done healing.");
                return;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 1: // prompt
            stream.log("heal_at_pokecenter: Detected prompt.");
            seen_prompt = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 2: // advance dialog
            stream.log("heal_at_pokecenter: Detected advance dialog.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 3: // tutorial
            stream.log("heal_at_pokecenter: Detected tutorial.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;   
        default:
            stream.log("heal_at_pokecenter: Timed out.");
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to heal at pokecenter.",
                stream
            );  
        }
    }
}


}
}
}
