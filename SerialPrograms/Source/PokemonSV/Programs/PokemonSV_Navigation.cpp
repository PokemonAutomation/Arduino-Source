/*  Navigation
 exit_tera_without_catching/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DateReader.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_MapDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_PicnicDetector.h"
#include "PokemonSV/Inference/PokemonSV_ZeroGateWarpPromptDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapPokeCenterIconDetector.h"
#include "PokemonSV_ConnectToInternet.h"
#include "PokemonSV_Navigation.h"

#include <cmath>

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

void open_map_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    {
        OverworldWatcher overworld(COLOR_CYAN);
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
        }
        else{
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

        OverworldWatcher overworld(COLOR_CYAN);
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
            }
            else{ // click R joystick to change to fixed view
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

void fly_to_overworld_from_map(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    // Press A to bring up the promp dialog on choosing "Fly here", "Set as destination", "Never mind".
    pbf_press_button(context, BUTTON_A, 20, 130);

    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(2)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "fly_to_overworld_from_map(): Failed to open map after 2 minutes.",
                true
            );
        }

        int ret = 0;
        {
            OverworldWatcher overworld(COLOR_CYAN);
            MapExitWatcher map(COLOR_RED);
            GradientArrowWatcher spot_dialog_watcher(COLOR_YELLOW, GradientArrowType::RIGHT, {0.469, 0.500, 0.215, 0.150});
            PromptDialogWatcher confirm_watcher(COLOR_BLUE, {0.686, 0.494, 0.171, 0.163});

            context.wait_for_all_requests();
            ret = wait_until(
                console, context,
                std::chrono::minutes(2),
                {overworld, map, spot_dialog_watcher, confirm_watcher}
            );
        }
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            return;
        case 1:
            console.log("Detected map.");
            // Press A to bring up the promp dialog on choosing "Fly here", "Set as destination", "Never mind".
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 2:
            console.log("Detected fly here prompt dialog.");
            console.overlay().add_log("Fly", COLOR_WHITE);
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
        case 3:
            console.log("Detected fly confirmation prompt.");
            pbf_press_button(context, BUTTON_A, 20, 130);
            continue;
            
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

        OverworldWatcher overworld(COLOR_CYAN);
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
    OverworldWatcher overworld(COLOR_CYAN);
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

        OverworldWatcher overworld(COLOR_CYAN);
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
    OverworldWatcher overworld(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, BUTTON_B, 20, 230);
            }
        },
        {overworld}
    );
    if (ret < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "leave_box_system_to_overworld(): Unknown state after 10 button B presses.",
            true
        );
    }
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

        OverworldWatcher overworld(COLOR_CYAN);
        MapExitWatcher pokedex(COLOR_RED);

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
    LetsGoKillWatcher menu(console.logger(), COLOR_RED, true, {0.23, 0.23, 0.04, 0.08});
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
    } else {
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "open_recently_battled_from_pokedex(): Unknown state after 10 dpad down presses.",
            true
        );
    }
}


void leave_phone_to_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Exiting to overworld from Rotom Phone...");
    OverworldWatcher overworld(COLOR_CYAN);
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


void fly_to_closest_pokecenter_on_map(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    // Zoom in one level onto the map.
    // If the player character icon or any wild pokemon icon overlaps with the PokeCenter icon, the code cannot
    // detect it. So we zoom in as much as we can to prevent any icon overlap.
    pbf_press_button(context, BUTTON_ZR, 20, 100);
    context.wait_for_all_requests();
    const auto snapshot_frame = console.video().snapshot().frame;
    const size_t screen_width = snapshot_frame->width();
    const size_t screen_height = snapshot_frame->height();

    double closest_icon_x = 0., closest_icon_y = 0.;
    double max_dist = 0.0;
    const double center_x = 0.5 * screen_width, center_y = 0.5 * screen_height;
    {
        MapPokeCenterIconWatcher pokecenter_watcher(COLOR_RED, console.overlay(), MAP_READABLE_AREA);
        int ret = wait_until(console, context, std::chrono::seconds(10), {pokecenter_watcher});
        if (ret != 0){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "fly_to_closest_pokecenter_on_map(): Cannot detect PokeCenter icon after 10 seconds",
                true
            );
        }
        // Find the detected PokeCenter icon closest to the screen center (where player character is on the map).
        for(const auto& box: pokecenter_watcher.found_locations()){
            const double loc_x = (box.x + box.width/2) * screen_width;
            const double loc_y = (box.y + box.height/2) * screen_height;
            const double x_diff = loc_x - center_x, y_diff = loc_y - center_y;
            const double dist = x_diff * x_diff + y_diff * y_diff;
            if (max_dist < dist){
                max_dist = dist;
                closest_icon_x = loc_x; closest_icon_y = loc_y;
            }
        }
        console.log("Found closest pokecenter icon on map: (" + std::to_string(closest_icon_x) + ", " + std::to_string(closest_icon_y) + ").");
    }

    // Convert the vector from center to the PokeCenter icon into a left joystick movement
    const double dif_x = closest_icon_x - center_x;
    const double dif_y = closest_icon_y - center_y;
    const double magnitude = std::max(std::sqrt(max_dist), 1.0);
    const double push_x = dif_x * 64 / magnitude, push_y = dif_y * 64 / magnitude;

    // 0.5 is too large, 0.25 a little too small
    const double scale = 0.30;

    const uint8_t move_x = uint8_t(std::max(std::min(int(round(push_x + 128) + 0.5), 255), 0));
    const uint8_t move_y = uint8_t(std::max(std::min(int(round(push_y + 128) + 0.5), 255), 0));

    console.overlay().add_log("Move Cursor to PokeCenter", COLOR_WHITE);
    pbf_move_left_joystick(context, move_x, move_y, uint16_t(magnitude * scale), 30);
    fly_to_overworld_from_map(info, console, context);
}


}
}
}
