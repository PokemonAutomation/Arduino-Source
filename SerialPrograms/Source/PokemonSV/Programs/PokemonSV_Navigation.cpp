/*  Navigation
 exit_tera_without_catching/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DateReader.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapMenuDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapPokeCenterIconDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_PicnicDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_ZeroGateWarpPromptDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV_ConnectToInternet.h"
#include "PokemonSV_Navigation.h"

#include <array>
#include <cmath>
#include <sstream>
#include <cfloat>
#include <iostream>
namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


void set_time_to_12am_from_home(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    DateReader reader;
    VideoOverlaySet overlays(console.overlay());
    reader.make_overlays(overlays);

//    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
    home_to_date_time(context, true, false);
    pbf_press_button(context, BUTTON_A, 20, 50);
    reader.set_hours(info, console, context, 0);
    pbf_press_button(context, BUTTON_A, 20, 30);
    pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
//    resume_game_from_home(console, context);
}

void day_skip_from_overworld(ConsoleHandle& console, BotBaseContext& context){
    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
    home_to_date_time(context, true, true);
    ssf_press_button(context, BUTTON_A, 20, 10);
    ssf_issue_scroll(context, DPAD_RIGHT, 0);
    ssf_press_button(context, BUTTON_A, 2);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_press_button(context, BUTTON_A, 0);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 3);
    ssf_issue_scroll(context, SSF_SCROLL_RIGHT, 0);
    ssf_press_button(context, BUTTON_A, 20, 10);
    pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
    resume_game_from_home(console, context);
}

void press_Bs_to_back_to_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, uint16_t seconds_between_b_presses){
    context.wait_for_all_requests();
    OverworldWatcher overworld(console, COLOR_RED);
    int ret = run_until(
        console, context,
        [seconds_between_b_presses](BotBaseContext& context){
            pbf_wait(context, seconds_between_b_presses * TICKS_PER_SECOND); // avoiding pressing B if already in overworld
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, BUTTON_B, 20, seconds_between_b_presses * TICKS_PER_SECOND);
            }
        },
        {overworld}
    );
    if (ret < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "press_Bs_to_back_to_overworld(): Unable to detect overworld after 10 button B presses.",
            true
        );
    }
}

void open_map_from_overworld(
    const ProgramInfo& info,
    ConsoleHandle& console, 
    BotBaseContext& context,
    bool clear_tutorial
){
    {
        OverworldWatcher overworld(console, COLOR_CYAN);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(10),
            {overworld}
        );
        context.wait_for(std::chrono::milliseconds(100));
        if (ret == 0){
            console.log("Detected overworld.");
            pbf_press_button(context, BUTTON_Y, 20, 105); // open map
        }else{
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "open_map_from_overworld(): No overworld state found after 10 seconds.",
                true
            );
        }
    }

    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(1)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "open_map_from_overworld(): Failed to open map after 1 minute.",
                true
            );
        }

        OverworldWatcher overworld(console, COLOR_CYAN);
        AdvanceDialogWatcher advance_dialog(COLOR_YELLOW);
        PromptDialogWatcher prompt_dialog(COLOR_GREEN);
        MapWatcher map(COLOR_RED);

        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {overworld, advance_dialog, prompt_dialog, map}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            pbf_press_button(context, BUTTON_Y, 20, 105); // open map
            continue;
        case 1:
            console.log("Detected dialog. Did you fall down?", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 2:
            console.log("Detected dialog. Did you fall down?", COLOR_RED);
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 3:
            console.log("Detected map.");
            console.overlay().add_log("Map opened", COLOR_WHITE);
            if (map.map_in_fixed_view()){
                return;
            }else{ // click R joystick to change to fixed view
                if (clear_tutorial){
                    pbf_press_button(context, BUTTON_A, 20, 105);
                }
                console.log("Map in rotate view, fix it");
                console.overlay().add_log("Change map to fixed view", COLOR_WHITE);
                pbf_press_button(context, BUTTON_RCLICK, 20, 105);
                continue;
            }
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "open_map_from_overworld(): No recognized state after 30 seconds.",
                true
            );
        }
    }
}

bool fly_to_overworld_from_map(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, bool check_fly_menuitem){
    context.wait_for_all_requests();
    // Press A to bring up the promp dialog on choosing "Fly here", "Set as destination", "Never mind".
    pbf_press_button(context, BUTTON_A, 20, 130);

    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(2)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "fly_to_overworld_from_map(): Failed to fly from map after 2 minutes.",
                true
            );
        }

        int ret = 0;
        OverworldWatcher overworld(console, COLOR_CYAN);
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

        ret = wait_until(console, context, std::chrono::minutes(2), callbacks);
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected overworld. Fly successful.");
            return true;
        case 1:
            console.log("Detected map. Pressing A to open map menu.");
            // Press A to bring up the promp dialog on choosing "Fly here", "Set as destination", "Never mind".
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 2:
            console.log("Detected fly here prompt dialog.");
            console.overlay().add_log("Fly");
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 3:
            console.log("Detected fly confirmation prompt.");
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 4:
            console.log("Detected no fly spot here.");
            console.overlay().add_log("No fly spot", COLOR_RED);
            return false;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "fly_to_overworld_from_map(): No recognized state after 2 minutes.",
                true
            );
        }
    }
}


void picnic_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Start picnic from overworld...");
    WallClock start = current_time();
    bool success = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(3)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "picnic_from_overworld(): Failed to start picnic after 3 minutes.",
                true
            );
        }

        OverworldWatcher overworld(console, COLOR_CYAN);
        MainMenuWatcher main_menu(COLOR_RED);
        PicnicWatcher picnic;
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {overworld, main_menu, picnic}
        );
        context.wait_for(std::chrono::milliseconds(100));
        const bool fast_mode = false;
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            pbf_press_button(context, BUTTON_X, 20, 105); // open menu 
            continue;
        case 1:
            console.log("Detected main menu.");
            success = main_menu.move_cursor(info, console, context, MenuSide::RIGHT, 2, fast_mode);
            if (success == false){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "picnic_from_overworld(): Cannot move menu cursor to picnic.",
                    true
                );
            }
            pbf_mash_button(context, BUTTON_A, 125); // mash button A to enter picnic mode
            continue;
        case 2:
            console.log("Detected picnic.");
            console.overlay().add_log("Start picnic", COLOR_WHITE);
            // extra wait to make sure by the end the player can move.
            // the player throwing out pokeballs animation is long.
            pbf_wait(context, 1000);
            context.wait_for_all_requests();
            return;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "picnic_from_overworld(): No recognized state after 30 seconds.",
                true
            );
        }
    }
}

void leave_picnic(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Leaving picnic...");
    console.overlay().add_log("Leaving picnic", COLOR_WHITE);

    pbf_press_button(context, BUTTON_Y, 30, 100);
    for(int i = 0; i < 5; i++){
        PromptDialogWatcher prompt(COLOR_RED, {0.595, 0.517, 0.273, 0.131});
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(5),
            {prompt}
        );

        if (ret == 0){
            console.log("Detected end picnic prompt");
            break;
        }

        if (i == 4){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "leave_picnic(): Failed to leave picnic after 5 tries.",
                true
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
    OverworldWatcher overworld(console, COLOR_CYAN);
    int ret = wait_until(
        console, context,
        std::chrono::seconds(20),
        {overworld}
    );
    if (ret < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "leave_picnic(): Failed to detecxt overworld after 20 seconds.",
            true
        );
    }
    // Wait three more seconds to make sure the player character is free to operate:
    context.wait_for(std::chrono::seconds(3));
}


void enter_box_system_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Enter box system from overworld...");
    WallClock start = current_time();
    bool success = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(3)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "enter_box_system_from_overworld(): Failed to enter box system after 3 minutes.",
                true
            );
        }

        OverworldWatcher overworld(console, COLOR_CYAN);
        MainMenuWatcher main_menu(COLOR_RED);
        GradientArrowWatcher box_slot_one(COLOR_BLUE, GradientArrowType::DOWN, {0.24, 0.16, 0.05, 0.09});
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {overworld, main_menu, box_slot_one}
        );
        context.wait_for(std::chrono::milliseconds(100));
        const bool fast_mode = false;
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            pbf_press_button(context, BUTTON_X, 20, 105); // open menu
            continue;
        case 1:
            console.log("Detected main menu.");
            console.overlay().add_log("Enter box", COLOR_WHITE);
            success = main_menu.move_cursor(info, console, context, MenuSide::RIGHT, 1, fast_mode);
            if (success == false){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "enter_box_system_from_overworld(): Cannot move menu cursor to Boxes.",
                    true
                );
            }
            pbf_press_button(context, BUTTON_A, 20, 50);
            continue;
        case 2:
            console.log("Detected box.");
            context.wait_for(std::chrono::milliseconds(200));
            return;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "enter_box_system_from_overworld(): No recognized state after 30 seconds.",
                true
            );
        }
    }
}


void leave_box_system_to_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Leave box system to overworld...");
    press_Bs_to_back_to_overworld(info, console, context);
}


void open_pokedex_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Opening Pokédex...");
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(30)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "open_pokedex_from_overworld(): Failed to open Pokédex after 30 seconds.",
                true
            );
        }

        OverworldWatcher overworld(console, COLOR_CYAN);
        WhiteButtonWatcher pokedex(COLOR_RED, WhiteButton::ButtonY, {0.800, 0.118, 0.030, 0.060});

        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {overworld, pokedex}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            // Try opening the Pokédex if overworld is detected
            pbf_press_button(context, BUTTON_MINUS, 20, 100);
            continue;
        case 1:
            console.log("Detected Pokédex.");
            return;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "open_pokedex_from_overworld(): No recognized state after 30 seconds.",
                true
            );
        }
    }
}


void open_recently_battled_from_pokedex(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Opening recently battled...");
    LetsGoKillWatcher menu(console.logger(), COLOR_RED, true, {0.23, 0.23, 0.04, 0.20});
    context.wait_for_all_requests();

    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            for (size_t i = 0; i < 10; i++){
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
            }
        },
        {menu}
    );
    if (ret == 0){
        console.log("Detected Recently Battled menu icon.");
        pbf_mash_button(context, BUTTON_A, 150);
        pbf_wait(context, 200);
    }else{
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "open_recently_battled_from_pokedex(): Unknown state after 10 dpad down presses.",
            true
        );
    }
}


void leave_phone_to_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Exiting to overworld from Rotom Phone...");
    OverworldWatcher overworld(console, COLOR_CYAN);
    context.wait_for_all_requests();

    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            for (size_t i = 0; i < 10; i++){
                pbf_press_button(context, BUTTON_Y, 20, 1000);
            }
        },
        {overworld}
    );
    if (ret < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "leave_phone_to_overworld(): Unknown state after 10 button Y presses.",
            true
        );
    }
}

// While in the current map zoom level, detect pokecenter icons and move the map cursor there.
// Return true if succeed. Return false if no visible pokcenter on map
bool detect_closest_pokecenter_and_move_map_cursor_there(
    const ProgramInfo& info,
    ConsoleHandle& console, 
    BotBaseContext& context,
    double push_scale
){
    context.wait_for_all_requests();
    const auto snapshot_frame = console.video().snapshot().frame;
    const size_t screen_width = snapshot_frame->width();
    const size_t screen_height = snapshot_frame->height();

    double closest_icon_x = 0., closest_icon_y = 0.;
    double max_dist = DBL_MAX;
    const double center_x = 0.5 * screen_width, center_y = 0.5 * screen_height;
    {
        MapPokeCenterIconWatcher pokecenter_watcher(COLOR_RED, console.overlay(), MAP_READABLE_AREA);
        int ret = wait_until(console, context, std::chrono::seconds(2), {pokecenter_watcher});
        if (ret != 0){
            console.log("No visible pokecetner found on map");
            console.overlay().add_log("No whole PokeCenter icon");
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
            console.log(os.str());

            if (dist2 < max_dist){
                max_dist = dist2;
                closest_icon_x = loc_x; closest_icon_y = loc_y;
            }
        }
        console.log("Found closest pokecenter icon on map: (" + std::to_string(closest_icon_x) + ", " + std::to_string(closest_icon_y) + ").");
        console.overlay().add_log("Detected PokeCenter icon");
    }

    // Convert the vector from center to the PokeCenter icon into a left joystick movement
    const double dif_x = closest_icon_x - center_x;
    const double dif_y = closest_icon_y - center_y;
    const double magnitude = std::max(std::sqrt(max_dist), 1.0);
    const double push_x = dif_x * 64 / magnitude, push_y = dif_y * 64 / magnitude;

    // 0.5 is too large, 0.25 a little too small, 0.30 is a bit too much for a far-away pokecenter
    const double scale = push_scale;

    const uint8_t move_x = uint8_t(std::max(std::min(int(round(push_x + 128) + 0.5), 255), 0));
    const uint8_t move_y = uint8_t(std::max(std::min(int(round(push_y + 128) + 0.5), 255), 0));

    console.overlay().add_log("Move Cursor to PokeCenter", COLOR_WHITE);
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
    ConsoleHandle& console, 
    BotBaseContext& context, 
    double push_scale
){

    if (!detect_closest_pokecenter_and_move_map_cursor_there(info, console, context, push_scale)){
        return false;
    }
    bool check_fly_menuitem = true;
    const bool success = fly_to_overworld_from_map(info, console, context, check_fly_menuitem);
    if (success){
        return true;
    }
    else {
        // detected pokecenter, but failed to fly there.
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "fly_to_visible_closest_pokecenter_cur_zoom_level(): Detected pokecenter, but failed to fly there as no \"Fly\" menuitem.",
            true
        );
    }

}


void fly_to_closest_pokecenter_on_map(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    const int max_try_count = 6;
    int try_count = 0;
    // Part 1: Tries to detect a pokecenter that is very close to the player
    // Zoom in one level onto the map.
    // If the player character icon or any wild pokemon icon overlaps with the PokeCenter icon, the code cannot
    // detect it. So we zoom in as much as we can to prevent any icon overlap.
    const std::array<double, max_try_count + 1> adjustment_table =  {1, 1, 1, 1.1, 1.2, 0.9, 0.8};
    
    while(true){
        try {
            pbf_press_button(context, BUTTON_ZR, 40, 100);
            // try different magnitudes of cursor push with each failure.
            double push_scale = 0.29 * adjustment_table[try_count];
            // std::cout << "push_scale: " << std::to_string(push_scale) << std::endl;
            if (fly_to_visible_closest_pokecenter_cur_zoom_level(info, console, context, push_scale)){
                return; // success in finding the closest pokecenter. Return.
            }

            // no visible pokecenters at this zoom level. Move on to part 2.
            break;
        }
        catch (OperationFailedException& e){ // pokecenter was detected, but failed to fly there
            try_count++;
            if (try_count > max_try_count){
                throw e;
            }
            console.log("Failed to find the fly menuitem. Restart the closest Pokecenter travel process.");
            press_Bs_to_back_to_overworld(info, console, context);
            open_map_from_overworld(info, console, context);
        }
    }

    // Part 2: Tries to detect any pokecenter that is overlapped with the player.
    // Zoom out to the max warpable level and try pressing on the player character.
    console.log("Zoom to max map level to try searching for Pokecenter again.");
    console.overlay().add_log("Pokecenter Icon occluded");
    pbf_press_button(context, BUTTON_ZL, 40, 100);
    pbf_press_button(context, BUTTON_ZL, 40, 100);

    const bool check_fly_menuitem = true;
    if (fly_to_overworld_from_map(info, console, context, check_fly_menuitem)){
        return; // success in flying to the pokecenter that overlaps with the player character at max warpable level.
    }

    // Failed to find pokecenter overlapping with player
    console.log("No PokeCenter icon overlapping with the player character on the max warpable level");
    console.overlay().add_log("No overlapping PokeCenter");
    // press B to close the destination menu item
    pbf_press_button(context, BUTTON_B, 60, 100);


    // Part 3: Tries to detect a pokecenter that is further away from the player, while at max warpable level
    try_count = 0;
    while(true){
        try {
            double push_scale = 0.29 * adjustment_table[try_count];
            // std::cout << "push_scale: " << std::to_string(push_scale) << std::endl;
            // Now try finding the closest pokecenter at the max warpable level
            if (fly_to_visible_closest_pokecenter_cur_zoom_level(info, console, context, push_scale)){
                return; // success in finding the closest pokecenter. Return.
            }
            else {
                // Does not detect any pokecenter on map
                console.overlay().add_log("Still no PokeCenter Found!", COLOR_RED);
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "fly_to_closest_pokecenter_on_map(): At max warpable map level, still cannot find PokeCenter icon.",
                    true
                );
            }
        }
        catch (OperationFailedException& e){ // pokecenter was detected, but failed to fly there
            try_count++;
            if (try_count > max_try_count){
                throw e;
            }
            console.log("Failed to find the fly menuitem. Restart the closest Pokecenter travel process.");
            press_Bs_to_back_to_overworld(info, console, context);
            open_map_from_overworld(info, console, context);
            // zoom out to max warpable level
            pbf_press_button(context, BUTTON_ZL, 40, 100);
        }
    }



    
    
   
}

void jump_off_wall_until_map_open(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    for (auto i = 0; i < 3; i++){
        pbf_press_button(context, BUTTON_L, 50, 50);
        pbf_press_button(context, BUTTON_B, 50, 50);
        pbf_move_left_joystick(context, 128, 255, 100, 50);
        context.wait_for_all_requests();
        try{
            open_map_from_overworld(info, console, context);
            break;
        }
        catch(...){
            console.log("Failed to open map.");
        }
        if (i >= 3){
            console.log("Could not escape wall.");
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "jump_off_wall_until_map_open(): Could not escape wall.",
                true
            );
        }
    }
}

// Open map and teleport back to town pokecenter to reset the hunting path.
void reset_to_pokecenter(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    open_map_from_overworld(info, console, context);
    fly_to_closest_pokecenter_on_map(info, console, context);
}

void realign_player(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    PlayerRealignMode realign_mode,
    uint8_t move_x, uint8_t move_y, uint8_t move_duration
){
    console.log("Realigning player direction...");

    switch (realign_mode){
    case PlayerRealignMode::REALIGN_NEW_MARKER:
        console.log("Setting new map marker...");
        open_map_from_overworld(info, console, context);
        pbf_press_button(context, BUTTON_ZR, 20, 105);
        pbf_move_left_joystick(context, move_x, move_y, move_duration, 1 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        leave_phone_to_overworld(info, console, context);
        break;
    case PlayerRealignMode::REALIGN_OLD_MARKER:
        open_map_from_overworld(info, console, context);
        leave_phone_to_overworld(info, console, context);
        pbf_press_button(context, BUTTON_L, 20, 105);
        break;
    case PlayerRealignMode::REALIGN_NO_MARKER:
        pbf_move_left_joystick(context, move_x, move_y, move_duration, 1 * TICKS_PER_SECOND);
        pbf_press_button(context, BUTTON_L, 20, 105);
        break;
    }
}


void walk_forward_until_dialog(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    NavigationMovementMode movement_mode,
    uint16_t seconds_timeout,
    uint8_t y
){

    DialogBoxWatcher        dialog(COLOR_RED, true);
    context.wait_for_all_requests();
    int ret = run_until(
        console, context,
        [&](BotBaseContext& context){
            ssf_press_left_joystick(context, 128, y, 0, seconds_timeout * TICKS_PER_SECOND);
            if (movement_mode == NavigationMovementMode::DIRECTIONAL_ONLY){
                pbf_wait(context, seconds_timeout * TICKS_PER_SECOND);
            } else if (movement_mode == NavigationMovementMode::DIRECTIONAL_SPAM_A){
                for (size_t j = 0; j < seconds_timeout; j++){
                    pbf_press_button(context, BUTTON_A, 20, 105);
                }
            }
        },
        {dialog}
    );
    context.wait_for(std::chrono::milliseconds(100));

    switch (ret){
    case 0: // dialog
        console.log("walk_forward_until_dialog(): Detected dialog.");
        return;
    default:
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "walk_forward_until_dialog(): Timed out. Did not detect dialog.",
            true
        );
    }
}

void walk_forward_while_clear_front_path(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
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
            console.log("walk_forward_while_clear_front_path() ticks traveled: " + std::to_string(forward_ticks));
            break;
        }

        pbf_move_left_joystick(context, 128, y, ticks_between_lets_go, 20);
        num_ticks_left -= ticks_between_lets_go;

        context.wait_for_all_requests();
        console.log("walk_forward_while_clear_front_path() ticks traveled: " + std::to_string(forward_ticks - num_ticks_left));

        pbf_press_button(context, BUTTON_R, 20, delay_after_lets_go);
        

    }
}

void mash_button_till_overworld(
    ConsoleHandle& console, 
    BotBaseContext& context, 
    uint16_t button, uint16_t seconds_run
){
    OverworldWatcher overworld(console, COLOR_CYAN);
    context.wait_for_all_requests();

    int ret = run_until(
        console, context,
        [button, seconds_run](BotBaseContext& context){
            ssf_mash1_button(context, button, seconds_run * TICKS_PER_SECOND);
            pbf_wait(context, seconds_run * TICKS_PER_SECOND);
        },
        {overworld}
    );

    if (ret < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "mash_button_till_overworld(): Timed out, no recognized state found.",
            true
        );
    }
}


bool attempt_fly_to_overlapping_flypoint(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context
){
    open_map_from_overworld(info, console, context);
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_ZL, 40, 100);

    return fly_to_overworld_from_map(info, console, context, true);
}

void fly_to_overlapping_flypoint(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    if (!attempt_fly_to_overlapping_flypoint(info, console, context)){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to reset to overlapping Pokecenter.",
            true
        );
    }
}

void confirm_no_overlapping_flypoint(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    if (attempt_fly_to_overlapping_flypoint(info, console, context)){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Overlapping fly detected, when it wasn't expected.",
            true
        );
    }
}

void enter_menu_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    int menu_index,
    MenuSide side,
    bool has_minimap
){
    if (!has_minimap){
        pbf_press_button(context, BUTTON_X, 20, 105);
    }

    WallClock start = current_time();
    bool success = false;

    while (true){
        if (current_time() - start > std::chrono::minutes(3)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "enter_menu_from_overworld(): Failed to enter specified menu after 3 minutes.",
                true
            );
        }

        OverworldWatcher overworld(console, COLOR_CYAN);
        MainMenuWatcher main_menu(COLOR_RED);
        context.wait_for_all_requests();

        int ret = run_until(
            console, context,
            [has_minimap](BotBaseContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_wait(context, 3 * TICKS_PER_SECOND);
                    if (!has_minimap){ 
                        // if no minimap, can't detect overworld, so repeatedly press X to cover for button drops
                        pbf_press_button(context, BUTTON_X, 20, 100);
                    }
                }
            },
            {overworld, main_menu}
        );
        context.wait_for(std::chrono::milliseconds(100));

        const bool fast_mode = false;
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            console.log("Detected main menu.");
            if (menu_index == -1){
                return;
            }
            success = main_menu.move_cursor(info, console, context, side, menu_index, fast_mode);
            if (success == false){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "enter_menu_from_overworld(): Cannot move menu cursor to specified menu.",
                    true
                );
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            return;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "enter_menu_from_overworld(): No recognized state after 30 seconds.",
                true
            );
        }
    }
}

void press_button_until_gradient_arrow(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    ImageFloatBox box_area_to_check,
    uint16_t button,
    GradientArrowType arrow_type
){
    GradientArrowWatcher arrow(COLOR_RED, arrow_type, box_area_to_check);
    int ret = run_until(
        console, context,
        [button](BotBaseContext& context){
            pbf_wait(context, 3 * TICKS_PER_SECOND); // avoid pressing button if arrow already detected
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, button, 20, 3 * TICKS_PER_SECOND);
            }
        },
        {arrow}
    );
    if (ret == 0){
        console.log("Gradient arrow detected.");
    }else{
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to detect gradient arrow.",
            true
        );
    }     
}

void basic_menu_navigation(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context,
    ImageFloatBox arrow_box_start,
    ImageFloatBox arrow_box_end,
    uint8_t dpad_button,
    uint16_t num_button_presses
){
    GradientArrowWatcher arrow_start(COLOR_RED, GradientArrowType::RIGHT, arrow_box_start);

    int ret = wait_until(console, context, Milliseconds(5000), { arrow_start });
    if (ret < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "basic_menu_navigation: Failed to detect gradient arrow at expected start position.",
            true
        );        
    }

    for (uint16_t i = 0; i < num_button_presses; i++){
        pbf_press_dpad(context, dpad_button, 20, 105);
    }

    GradientArrowWatcher arrow_end(COLOR_RED, GradientArrowType::RIGHT, arrow_box_end);
    ret = run_until(
        console,
        context,
        [dpad_button](BotBaseContext& context){
            for (int i = 0; i < 3; i++){
                pbf_press_dpad(context, dpad_button, 20, 500);
            }
        },
        { arrow_end }        
    );

    if (ret == 0){
        console.log("basic_menu_navigation: Desired item selected.");
    }else{
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "basic_menu_navigation: Failed to detect gradient arrow at expected end position.",
            true
        );        
    }
}


void heal_at_pokecenter(
    const ProgramInfo& info, 
    ConsoleHandle& console, 
    BotBaseContext& context
){
    context.wait_for_all_requests();
    
    if (!attempt_fly_to_overlapping_flypoint(info, console, context)){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to fly to pokecenter.",
            true
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
        OverworldWatcher    overworld(console, COLOR_CYAN);
        // TODO: test the Prompt watcher on all languages. Ensure FloatBox is sized correctly.
        PromptDialogWatcher prompt(COLOR_YELLOW, {0.50, 0.400, 0.400, 0.080}); // 0.630, 0.400, 0.100, 0.080 // {0.50, 0.40, 0.40, 0.50}
        AdvanceDialogWatcher    advance_dialog(COLOR_RED);
        TutorialWatcher     tutorial(COLOR_RED);
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::seconds(seconds_timeout),
            {overworld, prompt, advance_dialog, tutorial}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0: // overworld
            console.log("heal_at_pokecenter: Detected overworld.");
            if (seen_prompt){ 
                // if have seen the prompt dialog and are now in overworld, assume we have healed
                console.log("heal_at_pokecenter: Done healing.");
                return;
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 1: // prompt
            console.log("heal_at_pokecenter: Detected prompt.");
            seen_prompt = true;
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 2: // advance dialog
            console.log("heal_at_pokecenter: Detected advance dialog.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 3: // tutorial
            console.log("heal_at_pokecenter: Detected tutorial.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;            
        default:
            console.log("heal_at_pokecenter: Timed out.");
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "Failed to heal at pokecenter.",
                true
            );  
        }
    }


}


}
}
}
