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
#include "PokemonSV/Inference/Map/PokemonSV_FastTravelDetector.h"
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
using std::cout;
using std::endl;

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
        NormalBattleMenuWatcher battle(COLOR_BLUE);
        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        WhiteButtonWatcher map(COLOR_RED, WhiteButton::ButtonY, {0.800, 0.118, 0.030, 0.060});
        GradientArrowWatcher spot_dialog_watcher(COLOR_YELLOW, GradientArrowType::RIGHT, {0.469, 0.500, 0.215, 0.150});
        PromptDialogWatcher confirm_watcher(COLOR_BLUE, {0.686, 0.494, 0.171, 0.163});
        MapFlyMenuWatcher flyMenuItemWatcher(COLOR_BLACK);
        MapDestinationMenuWatcher destinationMenuItemWatcher(COLOR_BLACK);

        context.wait_for_all_requests();

        std::vector<PeriodicInferenceCallback> callbacks{overworld, map, spot_dialog_watcher, confirm_watcher, battle};
        if (check_fly_menuitem){ // callbacks: overworld, map, flyMenuItemWatcher, confirm_watcher, battle, destinationMenuItemWatcher
            callbacks[2] = flyMenuItemWatcher;
            callbacks.push_back(destinationMenuItemWatcher);
        }

        ret = wait_until(stream, context, std::chrono::minutes(2), callbacks);
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0: // overworld
            stream.log("Detected overworld. Fly successful.");
            return true;
        case 1: // map
            stream.log("Detected map. Pressing A to open map menu.");
            // Press A to bring up the promp dialog on choosing "Fly here", "Set as destination", "Never mind".
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 2: // spot_dialog_watcher or flyMenuItemWatcher
            stream.log("Detected fly here prompt dialog.");
            stream.overlay().add_log("Fly");
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 3: //confirm_watcher
            stream.log("Detected fly confirmation prompt.");
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 4: //battle
            stream.log("Detected battle.");
            throw_and_log<UnexpectedBattleException>(
                stream.logger(), ErrorReport::SEND_ERROR_REPORT,
                "fly_to_overworld_from_map(): Unexpectedly detected battle.",
                stream
            ); 
        case 5: //destinationMenuItemWatcher
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
            pbf_press_button(context, BUTTON_X, 160ms, 840ms); // open menu
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

std::string get_flypoint_string(FlyPoint fly_point){
    std::string fly_point_string;
    if (fly_point == FlyPoint::POKECENTER){
        fly_point_string = "Pokecenter";
    }else if(fly_point == FlyPoint::FAST_TRAVEL){
        fly_point_string = "Fast Travel";
    }

    return fly_point_string;
}

const std::vector<ImageFloatBox> get_flypoint_locations(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, FlyPoint fly_point){
    context.wait_for_all_requests();
    std::vector<ImageFloatBox> found_locations;
    MapPokeCenterIconWatcher pokecenter_watcher(COLOR_RED, stream.overlay(), MAP_READABLE_AREA);
    FastTravelWatcher fast_travel_watcher(COLOR_RED, stream.overlay(), MAP_READABLE_AREA);
    int ret = -1;
    if (fly_point == FlyPoint::POKECENTER){
        ret = wait_until(stream, context, std::chrono::seconds(2), {pokecenter_watcher});
        if (ret == 0){
            found_locations = pokecenter_watcher.found_locations();
        }
    }else if(fly_point == FlyPoint::FAST_TRAVEL){
        ret = wait_until(stream, context, std::chrono::seconds(2), {fast_travel_watcher});
        if (ret == 0){
            found_locations = fast_travel_watcher.found_locations();
        }
    }

    return found_locations;
}

void print_flypoint_location(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, FlyPoint fly_point){
    std::string fly_point_string = get_flypoint_string(fly_point);

    const std::vector<ImageFloatBox> found_locations = get_flypoint_locations(info, stream, context, fly_point);
    if (found_locations.empty()){
        stream.log("No visible " + fly_point_string + " found on map");
        return;
    }

    for(const auto& box: found_locations){
        std::ostringstream os;
        os << "Found " + fly_point_string + " at box: {" << box.x << ", " << box.y << "}"; // << ", width=" << box.width << ", height=" << box.height;
        stream.log(os.str());
  
    }
}



void place_marker_offset_from_flypoint(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    MoveCursor move_cursor_near_flypoint,
    FlyPoint fly_point, 
    ExpectedMarkerPosition marker_offset
){

    stream.log("place_marker_offset_from_flypoint()");
    WallClock start = current_time();

    while (true){
        if (current_time() - start > std::chrono::minutes(2)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "place_marker_offset_from_flypoint(): Failed to place down marker after 2 minutes.",
                stream
            );
        }

        try {
            open_map_from_overworld(info, stream, context, false);

            // move cursor near landmark (pokecenter)
            switch(move_cursor_near_flypoint.zoom_change){
            case ZoomChange::ZOOM_IN:
                pbf_press_button(context, BUTTON_ZR, 160ms, 840ms);
                break;
            case ZoomChange::ZOOM_IN_TWICE:
                pbf_press_button(context, BUTTON_ZR, 160ms, 840ms);
                pbf_press_button(context, BUTTON_ZR, 160ms, 840ms);
                break;                
            case ZoomChange::ZOOM_OUT:
                pbf_press_button(context, BUTTON_ZL, 160ms, 840ms);
                break;    
            case ZoomChange::ZOOM_OUT_TWICE:
                pbf_press_button(context, BUTTON_ZL, 160ms, 840ms);
                pbf_press_button(context, BUTTON_ZL, 160ms, 840ms);
                break;                  
            case ZoomChange::KEEP_ZOOM:
                break;
            }
            uint8_t move_x1 = move_cursor_near_flypoint.move_x;
            uint8_t move_y1 = move_cursor_near_flypoint.move_y;
            Milliseconds move_duration1 = move_cursor_near_flypoint.move_duration;
            pbf_move_left_joystick(context, move_x1, move_y1, move_duration1, 1000ms);

            move_cursor_to_position_offset_from_flypoint(info, stream, context, fly_point, {marker_offset.x, marker_offset.y});

            // place down marker
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            leave_phone_to_overworld(info, stream, context);

            return;      

        }catch (UnexpectedBattleException&){
            run_wild_battle_press_A(stream, context, BattleStopCondition::STOP_OVERWORLD);
        }catch (OperationFailedException&){
            // reset to overworld if failed to center on the pokecenter, and re-try
            leave_phone_to_overworld(info, stream, context);
        }
    }
}

void move_cursor_to_position_offset_from_flypoint(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, FlyPoint fly_point, ExpectedMarkerPosition marker_offset){
    // loop through flypoint locations. find the point that is closest to marker_offset, by distance x_diff^2 + y_diff^2
    // based on the closest point, move cursor based on x_diff and y_diff. do this again until x_diff/y_diff are within certain margins
    std::string fly_point_string = get_flypoint_string(fly_point);
    size_t MAX_ATTEMPTS = 20;
    for (size_t i = 0; i < MAX_ATTEMPTS; i++){
        const std::vector<ImageFloatBox> found_locations = get_flypoint_locations(info, stream, context, fly_point);
        if (found_locations.empty()){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "move_cursor_to_position_offset_from_flypoint(): No visible " + fly_point_string + " found on map",
                stream
            );
        }
        
        const double expected_x = marker_offset.x;
        const double expected_y = marker_offset.y;
        double closest_icon_x = 0.0; 
        double closest_icon_y = 0.0;
        double closest_dist2 = DBL_MAX;  // distance^2 in pixels

        for(const auto& box: found_locations){
            const double found_x = box.x;
            const double found_y = box.y;
            const double x_diff = (found_x - expected_x) * 1920;
            const double y_diff = (found_y - expected_y) * 1080;
            const double dist2 = x_diff * x_diff + y_diff * y_diff;
            

            if (dist2 < closest_dist2){
                closest_dist2 = dist2;
                closest_icon_x = found_x; 
                closest_icon_y = found_y;
            }
        }
        stream.log("Found closest " + fly_point_string + " icon on map: (" + std::to_string(closest_icon_x) + ", " + std::to_string(closest_icon_y) + ").");


        // Convert the vector from ExpectedMarkerPosition to the FlyPoint icon into a left joystick movement
        const double dif_x = (closest_icon_x - expected_x) * 1920;
        const double dif_y = (closest_icon_y - expected_y) * 1080;
        const double magnitude = std::max(std::sqrt(closest_dist2), 1.0);
        double push_x = dif_x * 64 / magnitude;
        double push_y = dif_y * 64 / magnitude;

        double scale = 0.29;
        if (closest_dist2 < 1000){ // if we're already very close to the target, reduce push velocity and push duration
            scale = 0.1;
            push_x *= 0.25;
            push_y *= 0.25;
        }

        if (closest_dist2 < 5){ // if we're very very close to the target, reduce push velocity and push duration even further
            push_x *= 0.5;
            push_y *= 0.5;
        }

        cout << "sqrt(closest_dist2): " << std::sqrt(closest_dist2) << endl;
        // cout << "push_x " << push_x << endl;
        // cout << "dif_x "<< dif_x << endl;
        // cout << "magnitude " << magnitude << endl;

        if (std::sqrt(closest_dist2) < 1.01){
            // return when we're close enough to the target
            break;
        }

        const uint8_t move_x = uint8_t(std::max(std::min(int(round(push_x + 128) + 0.5), 255), 0));
        const uint8_t move_y = uint8_t(std::max(std::min(int(round(push_y + 128) + 0.5), 255), 0));

        const uint16_t push_time = std::max(uint16_t(magnitude * scale + 0.5), uint16_t(3));
        pbf_move_left_joystick(context, move_x, move_y, push_time, 30);
        context.wait_for_all_requests();
    }

}

// While in the current map zoom level, detect pokecenter icons and move the map cursor there.
// Return true if succeed. Return false if no visible pokcenter on map
bool detect_closest_flypoint_and_move_map_cursor_there(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    FlyPoint fly_point,
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
        std::string fly_point_string = get_flypoint_string(fly_point);
        const std::vector<ImageFloatBox> found_locations = get_flypoint_locations(info, stream, context, fly_point);
        if (found_locations.empty()){
            stream.log("No visible " + fly_point_string + " found on map");
            stream.overlay().add_log("No whole " + fly_point_string + " icon");
            return false;
        }
        
        // Find the detected PokeCenter icon closest to the screen center (where player character is on the map).
        for(const auto& box: found_locations){
            const double loc_x = (box.x + box.width/2) * screen_width;
            const double loc_y = (box.y + box.height/2) * screen_height;
            const double x_diff = loc_x - center_x, y_diff = loc_y - center_y;
            const double dist2 = x_diff * x_diff + y_diff * y_diff;
            std::ostringstream os;
            os << "Found " + fly_point_string + " at box: x=" << box.x << ", y=" << box.y << ", width=" << box.width << ", height=" << box.height << 
                ", dist to center " << std::sqrt(dist2) << " pixels";
            stream.log(os.str());

            if (dist2 < max_dist){
                max_dist = dist2;
                closest_icon_x = loc_x; closest_icon_y = loc_y;
            }
        }
        stream.log("Found closest " + fly_point_string + " icon on map: (" + std::to_string(closest_icon_x) + ", " + std::to_string(closest_icon_y) + ").");
        stream.overlay().add_log("Detected " + fly_point_string + " icon");
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
bool fly_to_visible_closest_flypoint_cur_zoom_level(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context, 
    FlyPoint fly_point,
    double push_scale
){
    if (!detect_closest_flypoint_and_move_map_cursor_there(info, stream, context, fly_point, push_scale)){
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
            "fly_to_visible_closest_flypoint_cur_zoom_level(): Detected pokecenter, but failed to fly there as no \"Fly\" menuitem.",
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
            if (fly_to_visible_closest_flypoint_cur_zoom_level(info, stream, context, FlyPoint::POKECENTER, push_scale)){
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
            if (fly_to_visible_closest_flypoint_cur_zoom_level(info, stream, context, FlyPoint::POKECENTER, push_scale)){
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
        pbf_press_button(context, BUTTON_L, 400ms, 400ms);
        pbf_press_button(context, BUTTON_B, 400ms, 400ms);
        pbf_move_left_joystick(context, 128, 255, 800ms, 400ms);
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



void walk_forward_until_dialog(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    NavigationMovementMode movement_mode,
    uint16_t seconds_timeout,
    uint8_t x,
    uint8_t y
){

    DialogBoxWatcher dialog(COLOR_RED, true);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            ssf_press_left_joystick(context, x, y, 0, seconds_timeout * TICKS_PER_SECOND);
            if (movement_mode == NavigationMovementMode::DIRECTIONAL_ONLY){
                pbf_wait(context, seconds_timeout * TICKS_PER_SECOND);
            } else if (movement_mode == NavigationMovementMode::DIRECTIONAL_SPAM_A){
                pbf_mash_button(context, BUTTON_A, seconds_timeout * TICKS_PER_SECOND);
                // for (size_t j = 0; j < seconds_timeout; j++){
                //     pbf_press_button(context, BUTTON_A, 160ms, 840ms);
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
    
    // if (!attempt_fly_to_overlapping_flypoint(info, stream, context)){
    //     OperationFailedException::fire(
    //         ErrorReport::SEND_ERROR_REPORT,
    //         "Failed to fly to pokecenter.",
    //         stream
    //     );  
    // }           
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
        PromptDialogWatcher prompt(COLOR_YELLOW, {0.50, 0.400, 0.400, 0.320}); // 0.50, 0.400, 0.400, 0.080  // 0.630, 0.400, 0.100, 0.080 // {0.50, 0.40, 0.40, 0.50}
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
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 1: // prompt
            stream.log("heal_at_pokecenter: Detected prompt.");
            seen_prompt = true;
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 2: // advance dialog
            stream.log("heal_at_pokecenter: Detected advance dialog.");
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 3: // tutorial
            stream.log("heal_at_pokecenter: Detected tutorial.");
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
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


// spam A button to choose the first move
// throw exception if wipeout or if your lead faints.
void run_battle_press_A(
    VideoStream& stream,
    ProControllerContext& context,
    BattleStopCondition stop_condition,
    std::unordered_set<CallbackEnum> enum_optional_callbacks,
    bool detect_wipeout
){
    int16_t num_times_seen_overworld = 0;
    size_t consecutive_move_select = 0;
    while (true){
        NormalBattleMenuWatcher battle(COLOR_BLUE);
        SwapMenuWatcher         fainted(COLOR_PURPLE);
        OverworldWatcher        overworld(stream.logger(), COLOR_CYAN);
        AdvanceDialogWatcher    dialog(COLOR_RED);
        DialogArrowWatcher dialog_arrow(COLOR_RED, stream.overlay(), {0.850, 0.820, 0.020, 0.050}, 0.8365, 0.846);
        GradientArrowWatcher next_pokemon(COLOR_BLUE, GradientArrowType::RIGHT, {0.50, 0.51, 0.30, 0.10});
        GradientArrowWatcher bag(COLOR_BLUE, GradientArrowType::RIGHT, {0.10, 0.10, 0.05, 0.90});
        MoveSelectWatcher move_select_menu(COLOR_YELLOW);
        GradientArrowWatcher select_move_target(COLOR_BLUE, GradientArrowType::DOWN, {0.38, 0.08, 0.25, 0.1});

        std::vector<PeriodicInferenceCallback> callbacks; 
        std::vector<CallbackEnum> enum_all_callbacks;
        //  mandatory callbacks: Battle, Overworld, Advance Dialog, Swap menu, Move select
        //  optional callbacks: DIALOG_ARROW, NEXT_POKEMON, SELECT_MOVE_TARGET, BATTLE_BAG

        // merge the mandatory and optional callbacks as a set, to avoid duplicates. then convert to vector
        std::unordered_set<CallbackEnum> enum_all_callbacks_set{CallbackEnum::BATTLE, CallbackEnum::OVERWORLD, CallbackEnum::ADVANCE_DIALOG, CallbackEnum::SWAP_MENU, CallbackEnum::MOVE_SELECT}; // mandatory callbacks
        enum_all_callbacks_set.insert(enum_optional_callbacks.begin(), enum_optional_callbacks.end()); // append the mandatory and optional callback sets together
        enum_all_callbacks.assign(enum_all_callbacks_set.begin(), enum_all_callbacks_set.end());

        for (const CallbackEnum& enum_callback : enum_all_callbacks){
            switch(enum_callback){
            case CallbackEnum::ADVANCE_DIALOG:
                callbacks.emplace_back(dialog);
                break;                
            case CallbackEnum::OVERWORLD:
                callbacks.emplace_back(overworld);
                break;
            case CallbackEnum::DIALOG_ARROW:
                callbacks.emplace_back(dialog_arrow);
                break;
            case CallbackEnum::BATTLE:
                callbacks.emplace_back(battle);
                break;
            case CallbackEnum::NEXT_POKEMON: // to detect the "next pokemon" prompt.
                callbacks.emplace_back(next_pokemon);
                break;
            case CallbackEnum::SWAP_MENU:  // detecting Swap Menu implies your lead fainted.
                callbacks.emplace_back(fainted);
                break;                     
            case CallbackEnum::MOVE_SELECT:
                callbacks.emplace_back(move_select_menu);
                break;
            case CallbackEnum::SELECT_MOVE_TARGET:
                callbacks.emplace_back(select_move_target);
                break;
            case CallbackEnum::BATTLE_BAG:
                callbacks.emplace_back(bag);
                break;
            default:
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "run_battle_press_A: Unknown callback requested.");
            }
        }        
        context.wait_for_all_requests();

        int ret = wait_until(
            stream, context,
            std::chrono::seconds(360),
            callbacks
        );
        context.wait_for(std::chrono::milliseconds(100));
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_battle_press_A(): Timed out. Did not detect expected stop condition.",
                stream
            );
        }        

        CallbackEnum enum_callback = enum_all_callbacks[ret];
        switch (enum_callback){
        case CallbackEnum::BATTLE: // battle
            stream.log("Detected battle menu.");
            consecutive_move_select = 0;
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case CallbackEnum::MOVE_SELECT:
            stream.log("Detected move select. Spam first move");
            consecutive_move_select++;
            select_top_move(stream, context, consecutive_move_select);
            break;
        case CallbackEnum::OVERWORLD: // overworld
            stream.log("Detected overworld, battle over.");
            num_times_seen_overworld++;
            if (stop_condition == BattleStopCondition::STOP_OVERWORLD){
                return;
            }
            if(num_times_seen_overworld > 30){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "run_battle_press_A(): Stuck in overworld. Did not detect expected stop condition.",
                    stream
                );  
            }            
            break;
        case CallbackEnum::ADVANCE_DIALOG: // advance dialog
            stream.log("Detected dialog.");

            if (detect_wipeout){
                context.wait_for_all_requests();
                WipeoutDetector wipeout;
                VideoSnapshot screen = stream.video().snapshot();
                // dump_snapshot(console);
                if (wipeout.detect(screen)){
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "run_battle_press_A(): Detected wipeout. All pokemon fainted.",
                        stream
                    );                
                }
            }

            if (stop_condition == BattleStopCondition::STOP_DIALOG){
                return;
            }
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case CallbackEnum::DIALOG_ARROW:  // dialog arrow
            stream.log("run_battle_press_A: Detected dialog arrow.");
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case CallbackEnum::NEXT_POKEMON:
            stream.log("run_battle_press_A: Detected prompt for bringing in next pokemon. Keep current pokemon.");
            pbf_mash_button(context, BUTTON_B, 100);
            break;
        case CallbackEnum::SWAP_MENU:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "run_battle_press_A(): Lead pokemon fainted.",
                stream
            );    
        case CallbackEnum::SELECT_MOVE_TARGET:
            stream.log("run_battle_press_A: Detected arrows to select move target. Press A.");
            pbf_mash_button(context, BUTTON_A, 100);
            break;
        case CallbackEnum::BATTLE_BAG:
            stream.log("run_battle_press_A: Detected Bag. Press B. Hold Dpad Up so cursor is back on 'Battle'.");
            pbf_mash_button(context, BUTTON_B, 100);
            pbf_press_dpad(context, DPAD_UP, 2000ms, 100ms);
            break;
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "run_battle_press_A: Unknown callback triggered.");
          
        }
    }
}

void run_trainer_double_battle_press_A(
    VideoStream& stream,
    ProControllerContext& context,
    BattleStopCondition stop_condition,
    std::unordered_set<CallbackEnum> enum_optional_callbacks,
    bool detect_wipeout
){
    enum_optional_callbacks.insert(CallbackEnum::SELECT_MOVE_TARGET);  // always check for the "Select Move target" arrow, for double battles
    run_battle_press_A(stream, context, stop_condition, enum_optional_callbacks, detect_wipeout);
}

void run_trainer_battle_press_A(
    VideoStream& stream,
    ProControllerContext& context,
    BattleStopCondition stop_condition,
    std::unordered_set<CallbackEnum> enum_optional_callbacks,
    bool detect_wipeout
){
    enum_optional_callbacks.insert(CallbackEnum::NEXT_POKEMON);  // always check for the "Next pokemon" prompt when in trainer battles
    run_battle_press_A(stream, context, stop_condition, enum_optional_callbacks, detect_wipeout);
}

void run_wild_battle_press_A(
    VideoStream& stream,
    ProControllerContext& context,
    BattleStopCondition stop_condition,
    std::unordered_set<CallbackEnum> enum_optional_callbacks,
    bool detect_wipeout
){
    enum_optional_callbacks.insert(CallbackEnum::BATTLE_BAG);  // check for Bag.
    run_battle_press_A(stream, context, stop_condition, enum_optional_callbacks, detect_wipeout);
}



void select_top_move(VideoStream& stream, ProControllerContext& context, size_t consecutive_move_select){
    if (consecutive_move_select > 3){
        // to handle case where move is disabled/out of PP/taunted
        stream.log("Failed to select a move 3 times. Choosing a different move.", COLOR_RED);
        pbf_press_dpad(context, DPAD_DOWN, 20, 40);
    }
    pbf_mash_button(context, BUTTON_A, 100);

}


}
}
}
