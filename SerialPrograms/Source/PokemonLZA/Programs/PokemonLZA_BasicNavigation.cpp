/*  Basic Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_DirectionArrowDetector.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_MapIconDetector.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_MapDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_MainMenuDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA_BasicNavigation.h"

#include <cmath>

namespace PokemonAutomation::NintendoSwitch::PokemonLZA{

bool save_game_to_menu(ConsoleHandle& console, ProControllerContext& context){

    bool seen_save_button = false;
    bool seen_saved_dialog = false;

    WallClock start = current_time();
    while (true){
        context.wait_for_all_requests();

        if (current_time() - start > std::chrono::seconds(120)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "save_game_to_menu(): Unable to save game after 2 minutes.",
                console
            );
        }

        OverworldPartySelectionWatcher overworld(COLOR_WHITE, &console.overlay());
        MainMenuWatcher main_menu(COLOR_YELLOW, &console.overlay());
        SelectionArrowWatcher save_button(
            COLOR_GREEN,
            &console.overlay(),
            SelectionArrowType::RIGHT,
            {0.287118, 0.638835, 0.064410, 0.108738}
        );
        BlueDialogWatcher dialog(COLOR_YELLOW, &console.overlay());

        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {
                overworld,
                dialog,
                main_menu,
                save_button,
            }
        );
        context.wait_for(100ms);
        switch (ret){
        case 0:
            console.log("Detected overworld...");
            pbf_press_button(context, BUTTON_X, 160ms, 240ms);
            continue;
        case 1:
            if (!seen_save_button){
                console.log("Detected dialog before save prompt. Unable to save.", COLOR_RED);
                pbf_press_button(context, BUTTON_B, 160ms, 240ms);
                return false;
            }

            console.log("Detected dialog...");
            seen_saved_dialog = true;
            pbf_press_button(context, BUTTON_B, 160ms, 240ms);
            continue;
        case 2:
            console.log("Detected main menu...");
            if (seen_save_button && seen_saved_dialog){
                return true;
            }
            pbf_press_button(context, BUTTON_R, 160ms, 240ms);
            continue;
        case 3:
            console.log("Detected save button...");
            seen_save_button = true;
            pbf_press_button(context, BUTTON_A, 160ms, 240ms);
            continue;
        }
    }
}





bool open_map(ConsoleHandle& console, ProControllerContext& context, bool zoom_to_max){
    pbf_press_button(context, BUTTON_PLUS, 240ms, 40ms);
    context.wait_for_all_requests();
    console.log("Opening Map...");
    console.overlay().add_log("Open Map");
    
    WallClock deadline = current_time() + 30s;

    const ImageFloatBox icon_region{0.0, 0.089, 1.0, 0.911};
    MapIconDetector pokecenter_icon(COLOR_RED, MapIconType::PokemonCenter, icon_region, &console.overlay());
    MapIconDetector flyable_building_icon(COLOR_BLACK, MapIconType::BuildingFlyable, icon_region, &console.overlay());
    MapIconDetector flayble_cafe_icon(COLOR_ORANGE, MapIconType::CafeFlyable, icon_region, &console.overlay());
    MapWatcher map_detector(COLOR_RED, &console.overlay());
    map_detector.attach_map_icon_detector(pokecenter_icon);
    map_detector.attach_map_icon_detector(flyable_building_icon);
    map_detector.attach_map_icon_detector(flayble_cafe_icon);

    do{
        map_detector.reset_state();

        int ret = wait_until(
            console, context,
            5000ms,
            {map_detector}
        );
        switch (ret){
        case 0:
            console.log("Detected map!", COLOR_BLUE);
            console.overlay().add_log("Map Detected");

            if (zoom_to_max){
                map_detector.reset_state();
                // move right joystick to zoom out the map
                for(int i = 0; i < 3; i++){
                    pbf_move_right_joystick(context, 128, 255, 100ms, 300ms);
                }
                context.wait_for_all_requests();
                console.log("Set to fully zoomed out");
                console.overlay().add_log("Zoom to Max");
                // Re-run the map icon detectors to find flyable icons on the now fully
                // zoomed out map.
                wait_until(
                    console, context,
                    5000ms,
                    {map_detector}
                );
                return map_detector.detected_map_icons().size();
            }
            return map_detector.detected_map_icons().size() > 0;
        default:
            console.log("Map not found. Press + again", COLOR_ORANGE);
            pbf_press_button(context, BUTTON_PLUS, 240ms, 80ms);
            console.overlay().add_log("Map not Found. Press + Again");
            context.wait_for_all_requests();
        }

    }while (current_time() < deadline);

    console.overlay().add_log("Failed to Open Map After 30 sec", COLOR_RED);
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "open_map(): Unable to find map after 30 seconds.",
        console
    );
}

void open_hyperspace_map(ConsoleHandle& console, ProControllerContext& context){
    pbf_press_button(context, BUTTON_PLUS, 240ms, 40ms);
    context.wait_for_all_requests();
    console.log("Opening Hyperspace Map...");
    console.overlay().add_log("Open Hyperspace Map");
    
    WallClock deadline = current_time() + 30s;

    MapWatcher map_detector(COLOR_RED, &console.overlay());

    do{
        map_detector.reset_state();

        int ret = wait_until(
            console, context,
            5000ms,
            {map_detector}
        );
        switch (ret){
        case 0:
            console.log("Detected map!", COLOR_BLUE);
            console.overlay().add_log("Map Detected");
            return;
        default:
            console.log("Map not found. Press + again", COLOR_ORANGE);
            pbf_press_button(context, BUTTON_PLUS, 240ms, 80ms);
            console.overlay().add_log("Map not Found. Press + Again");
            context.wait_for_all_requests();
        }

    }while (current_time() < deadline);

    console.overlay().add_log("Failed to Open Hyperspace Map After 30 sec", COLOR_RED);
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "open_map(): Unable to find map after 30 seconds.",
        console
    );
}


FastTravelState fly_from_map(ConsoleHandle& console, ProControllerContext& context){
    console.log("Flying from map...");
    context.wait_for_all_requests();
    {
        BlackScreenWatcher start_flying(COLOR_RED);
        MapOverWatcher map_over(COLOR_RED, &console.overlay());
        BlueDialogWatcher blue_dialog(COLOR_BLUE, &console.overlay(), 50ms);
        int ret = run_until<ProControllerContext>(
            console, context,
            [&](ProControllerContext& context){
                for(int i = 0; i < 4; i++){
                    pbf_mash_button(context, BUTTON_A, 1000ms);
                }
            },
            {start_flying, map_over, blue_dialog,}
        );
        switch (ret){
        case 0:
        case 1:
            console.log("Flying from map... Started!");
            console.overlay().add_log("Fast Traveling");
            break;
        case 2:
            console.log("Spotted by wild pokemon, cannot fly");
            console.overlay().add_log("Spotted by Wild Pokemon");
            return FastTravelState::PURSUED;
        default:
            console.log("Map cursor not on fast travel location");
            console.overlay().add_log("Not On Fast Travel Location");
            return FastTravelState::NOT_AT_FLY_SPOT;
#if 0
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "fly_from_map(): Unable to fly.",
                console
            );
#endif
        }
    }

    OverworldPartySelectionWatcher overworld(COLOR_WHITE, &console.overlay());
    int ret = wait_until(
        console, context, 30s,  // set 30sec to be long enough for Switch 1 to load the overworld
        {overworld,}
    );
    switch (ret){
    case 0:
        console.log("Flying from map... Done!");
        console.overlay().add_log("Fast Travel Done");
        break;
    default:
        // return false;
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "fly_from_map(): Does not detect overworld after fast travel.",
            console
        );
    }

    return FastTravelState::SUCCESS;
}

FastTravelState open_map_and_fly_in_place(ConsoleHandle& console, ProControllerContext& context, bool zoom_to_max){
    bool can_fast_travel = open_map(console, context, zoom_to_max);
    if (!can_fast_travel){
        return FastTravelState::PURSUED;
    }
    return fly_from_map(console, context);
}


void move_map_cursor_from_entrance_to_zone(ConsoleHandle& console, ProControllerContext& context, WildZone zone){
    pbf_wait(context, 300ms);
    switch(zone){
    case WildZone::WILD_ZONE_1:
        pbf_move_left_joystick(context, 0, 150, 120ms, 0ms);
        break;
    case WildZone::WILD_ZONE_2:
        pbf_move_left_joystick(context, 120, 0, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_3:
        pbf_move_left_joystick(context, 128, 0, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_4:
        pbf_move_left_joystick(context, 255, 128, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_5:
        pbf_move_left_joystick(context, 170, 0, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_6:
        pbf_move_left_joystick(context, 80, 0, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_7:
        pbf_move_left_joystick(context, 0, 100, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_8:
        pbf_move_left_joystick(context, 0, 160, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_9:
        pbf_move_left_joystick(context, 70, 0, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_10:
        pbf_move_left_joystick(context, 255, 90, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_11:
        pbf_move_left_joystick(context, 0, 40, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_12:
        pbf_move_left_joystick(context, 20, 0, 150ms, 0ms);
        break;
    case WildZone::WILD_ZONE_13:
        pbf_move_left_joystick(context, 0, 160, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_14:
        pbf_move_left_joystick(context, 110, 255, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_15:
        pbf_move_left_joystick(context, 0, 0, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_16:
        pbf_move_left_joystick(context, 220, 0, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_17:
        pbf_move_left_joystick(context, 210, 0, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_18:
        pbf_move_left_joystick(context, 20, 255, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_19:
        pbf_move_left_joystick(context, 80, 255, 100ms, 0ms);
        break;
    case WildZone::WILD_ZONE_20_NO_DISTORTION:
        pbf_move_left_joystick(context, 0, 90, 140ms, 0ms);
        break;
    case WildZone::WILD_ZONE_20_WITH_DISTORTION:
        // During the distortion happening on top of Lumiose Tower as part
        // of the Mega Dimension DLC story, the wild zone 20 fast travel
        // symbol on the map is moved to the entrance gate. So we only
        // need a tiny left joystick push.
        pbf_move_left_joystick(context, 100, 100, 100ms, 0ms);
        break;
    }
    pbf_wait(context, 300ms);
}


void map_to_overworld(ConsoleHandle& console, ProControllerContext& context){
    OverworldPartySelectionWatcher overworld_watcher(COLOR_WHITE, &console.overlay());
    run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 2s);
            pbf_wait(context, 40s); // wait a long time in case day/night change immidiately happens
        },
        {{overworld_watcher}}
    );
    pbf_wait(context, 100ms); // wait 100ms for the game to give back control to player
    context.wait_for_all_requests();
}


void sit_on_bench(ConsoleHandle& console, ProControllerContext& context){
    context.wait_for_all_requests();
    console.overlay().add_log("Sitting on Bench");
    {
        BlackScreenOverWatcher black_screen(COLOR_BLUE);

        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                // mash A to start day/night change and into the transition animation
                pbf_mash_button(context, BUTTON_A, 26s);
            },
            {black_screen}
        );

        switch (ret){
        case 0:
            console.log("Detected day change.");
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "sit_on_bench(): No day/night transition detected after mashing A.",
                console
            );
        }
    }

    console.overlay().add_log("Waiting for New Day/Night");
    OverworldPartySelectionWatcher overworld(COLOR_WHITE, &console.overlay());
    int ret = wait_until(
        console, context,
        30s,
        {overworld}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "sit_on_bench(): Unable to go back to overworld after day/night change on bench after 30 seconds.",
            console
        );
    }
}


void wait_until_overworld(
    ConsoleHandle& console, ProControllerContext& context,
    std::chrono::milliseconds max_wait_time
){
    OverworldPartySelectionWatcher overworld(COLOR_WHITE, &console.overlay(), Milliseconds(100));
    DirectionArrowWatcher map_arrow(COLOR_BLUE, std::chrono::milliseconds(50));
    int ret = wait_until(
        console, context,
        max_wait_time,
        {overworld, map_arrow}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "wait_until_overworld(): Unable to detect overworld after " + 
                std::to_string(max_wait_time.count()) + " milliseconds.",
            console
        );
    }
    console.log("Detected overworld within " + std::to_string(max_wait_time.count()) + " milliseconds");
    context.wait_for(100ms); // extra 0.1 sec to let game give player control
}


double get_facing_direction(
    ConsoleHandle& console,
    ProControllerContext& context
){
    DirectionArrowWatcher arrow_watcher(COLOR_YELLOW, std::chrono::milliseconds(50));
    int ret = wait_until(
        console, context,
        std::chrono::seconds(40), // 40 sec to account for possible day/night change
        {arrow_watcher}
    );
    if (ret != 0){
        console.log("Direction arrow not detected within 1 second");
        console.overlay().add_log("No Minimap Arrow Found", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "get_facing_direction(): Direction arrow on minimap not detected within 40 second",
            console
        );
    }
    double angle = arrow_watcher.detected_angle_deg();
    console.log("Direction arrow detected! Angle: " + tostr_fixed(angle, 0) + " degrees");
    console.overlay().add_log("Minimap Arrow: " + tostr_fixed(angle, 0) + " deg", COLOR_YELLOW);
    return angle;
}

double get_angle_between_facing_directions(double dir1, double dir2){
    double angle = std::fabs(dir1 - dir2);
    if (angle > 180.0){
        angle = 360.0 - angle;
    }
    return angle;
}

bool leave_zone_gate(ConsoleHandle& console, ProControllerContext& context){
    console.log("Leaving zone gate");

    WallClock start_time = current_time();
    OverworldPartySelectionWatcher overworld_watcher(COLOR_WHITE, &console.overlay());
    pbf_mash_button(context, BUTTON_A, 1s);
    context.wait_for_all_requests();
    wait_until(
        console, context,
        std::chrono::seconds(40), // wait this long in case day/night change happens
        {overworld_watcher}
    );
    pbf_wait(context, 100ms); // after leaving the gate, the game needs this long time to give back control
    context.wait_for_all_requests();
    WallClock end_time = current_time();

    const auto duration = end_time - start_time;
    const auto second_count = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    // Due to day/night change may eating the mashing button A sequence, we may still be inside the zone!
    // We need to check if we can fast travel 
    if (duration < 16s){
        console.log("Leaving zone function took " + std::to_string(second_count) + " sec. No day/night change");
        console.overlay().add_log("Left Gate");

        // The animation of leaving the gate does not take more than 15 sec.
        // In this case, there is no day/night change. We are sure we are outside the wild zone
        // (unless some angry Garchomp or Drilbur used Dig and escaped wild zone containment... We don't
        // consider this case for now)
        return true;
    }

    console.log("Leaving zone function took " + std::to_string(second_count) + " sec. Day/night change happened");
    return false;
}


int run_towards_wild_zone_gate(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t run_direction_x, uint8_t run_direction_y,
    PokemonAutomation::Milliseconds run_time
){
    const ImageFloatBox button_A_box{0.3, 0.2, 0.4, 0.7};
    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, button_A_box, &console.overlay());
    OverworldPartySelectionOverWatcher overworld_gone(COLOR_WHITE, &console.overlay(), std::chrono::milliseconds(400));
    const int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            // running back
            ssf_press_button(context, BUTTON_B, 0ms, run_time, 0ms);
            pbf_move_left_joystick(context, run_direction_x, run_direction_y, run_time, 0ms);
        },
        {{buttonA, overworld_gone}}
    );
    switch (ret){
    case 0:
        console.log("Detected button A. Reached gate.");
        console.overlay().add_log("Reach Gate");
        return 0;
    case 1:
        console.log("Day/night change happened while escaping");
        console.overlay().add_log("Day/Night Change Detected");
        wait_until_overworld(console, context);
        return 1;
    default:
        console.log("Did not reach gate");
        return ret;
    }
}

int run_a_straight_path_in_overworld(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t direction_x, uint8_t direction_y,
    PokemonAutomation::Milliseconds duration
){
    OverworldPartySelectionOverWatcher overworld_gone(COLOR_WHITE, &console.overlay(), std::chrono::milliseconds(400));
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
            pbf_move_left_joystick(context, direction_x, direction_y, duration, 0ms);
        },
        {{overworld_gone}}
    );
    if (ret == 0){
        console.overlay().add_log("Day/Night Change Detected");
        wait_until_overworld(console, context);
    }
    return ret;
}


}
