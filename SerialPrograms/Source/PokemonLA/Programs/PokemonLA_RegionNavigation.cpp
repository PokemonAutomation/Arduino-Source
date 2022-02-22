/*  Region Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/Inference/PokemonLA_MapDetector.h"
#include "PokemonLA_RegionNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



void goto_professor(ConsoleHandle& console, Camp camp){
    switch (camp){
    case Camp::FIELDLANDS_FIELDLANDS:
        pbf_move_left_joystick(console, 255, 0, 125, 0);
        return;
    case Camp::FIELDLANDS_HEIGHTS:
        pbf_move_left_joystick(console, 240, 0, 200, 0);
        return;
    case Camp::MIRELANDS_MIRELANDS:
        pbf_move_left_joystick(console, 255, 64, 160, 0);
        return;
    case Camp::MIRELANDS_BOGBOUND:
        pbf_move_left_joystick(console, 255, 64, 140, 0);
        return;
    case Camp::COASTLANDS_BEACHSIDE:
        pbf_move_left_joystick(console, 255, 96, 125, 0);
        return;
    case Camp::COASTLANDS_COASTLANDS:
        pbf_move_left_joystick(console, 255, 48, 105, 0);
        return;
    case Camp::HIGHLANDS_HIGHLANDS:
        pbf_move_left_joystick(console, 255, 48, 176, 0);
        return;
    case Camp::HIGHLANDS_MOUNTAIN:
        pbf_move_left_joystick(console, 255, 32, 125, 0);
        return;
    case Camp::HIGHLANDS_SUMMIT:
        pbf_move_left_joystick(console, 255, 0, 125, 0);
        return;
    case Camp::ICELANDS_SNOWFIELDS:
        pbf_move_left_joystick(console, 255, 56, 125, 0);
        return;
    case Camp::ICELANDS_ICEPEAK:
        pbf_move_left_joystick(console, 255, 48, 75, 0);
        return;
    default:
        PA_THROW_StringException("Unknown Camp: " + std::to_string((int)camp));
    }
}


bool mash_A_to_change_region(ProgramEnvironment& env, ConsoleHandle& console){
    BlackScreenOverWatcher black_screen0;
    int ret = run_until(
        env, console,
        [](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 20 * TICKS_PER_SECOND);
        },
        { &black_screen0 }
    );
    if (ret < 0){
        console.log("Failed to load into region after 20 seconds.", COLOR_RED);
//        PA_THROW_StringException("Failed to load into region after 20 seconds.");
        return false;
    }
    env.wait_for(std::chrono::milliseconds(100));

    BlackScreenOverWatcher black_screen1a(COLOR_RED, {0.20, 0.02, 0.60, 0.05}, 150);
    BlackScreenOverWatcher black_screen1b(COLOR_RED, {0.20, 0.93, 0.60, 0.05}, 150);
    ret = wait_until(
        env, console,
        std::chrono::seconds(20),
        { &black_screen1a, &black_screen1b }
    );
    if (ret < 0){
        console.log("Failed to load into region after 20 seconds.", COLOR_RED);
//        PA_THROW_StringException("Failed to load into region after 20 seconds.");
        return false;
    }
    console.log("Loaded into map...");
    env.wait_for(std::chrono::seconds(1));
    return true;
}


bool goto_camp_from_jubilife(ProgramEnvironment& env, ConsoleHandle& console, Camp camp){
    //  Open the map.
    pbf_move_left_joystick(console, 128, 255, 200, 0);
    {
        MapDetector detector;
        int ret = run_until(
            env, console,
            [](const BotBaseContext& context){
                for (size_t c = 0; c < 10; c++){
                    pbf_press_button(context, BUTTON_A, 20, 105);
                }
            },
            { &detector }
        );
        if (ret < 0){
            console.log("Map not detected after 10 x A presses.", COLOR_RED);
            return false;
        }
        console.log("Found map!");
        env.wait_for(std::chrono::milliseconds(500));
    }

    MapRegion region;
    size_t slot;
    DpadPosition direction;
    switch (camp){
    case Camp::FIELDLANDS_FIELDLANDS:
        region = MapRegion::FIELDLANDS;
        slot = 0;
        direction = DPAD_RIGHT;
        break;
    case Camp::FIELDLANDS_HEIGHTS:
        region = MapRegion::FIELDLANDS;
        slot = 1;
        direction = DPAD_RIGHT;
        break;
    case Camp::MIRELANDS_MIRELANDS:
        region = MapRegion::MIRELANDS;
        slot = 0;
        direction = DPAD_RIGHT;
        break;
    case Camp::MIRELANDS_BOGBOUND:
        region = MapRegion::MIRELANDS;
        slot = 1;
        direction = DPAD_RIGHT;
        break;
    case Camp::COASTLANDS_BEACHSIDE:
        region = MapRegion::COASTLANDS;
        slot = 0;
        direction = DPAD_RIGHT;
        break;
    case Camp::COASTLANDS_COASTLANDS:
        region = MapRegion::COASTLANDS;
        slot = 1;
        direction = DPAD_RIGHT;
        break;
    case Camp::HIGHLANDS_HIGHLANDS:
        region = MapRegion::HIGHLANDS;
        slot = 0;
        direction = DPAD_LEFT;
        break;
    case Camp::HIGHLANDS_MOUNTAIN:
        region = MapRegion::HIGHLANDS;
        slot = 1;
        direction = DPAD_LEFT;
        break;
    case Camp::HIGHLANDS_SUMMIT:
        region = MapRegion::HIGHLANDS;
        slot = 2;
        direction = DPAD_LEFT;
        break;
    case Camp::ICELANDS_SNOWFIELDS:
        region = MapRegion::ICELANDS;
        slot = 0;
        direction = DPAD_LEFT;
        break;
    case Camp::ICELANDS_ICEPEAK:
        region = MapRegion::ICELANDS;
        slot = 1;
        direction = DPAD_LEFT;
        break;
    default:
        PA_THROW_StringException("Invalid Camp Enum: " + std::to_string((int)camp));
    }


    //  Move to region.
    MapRegion current_region = MapRegion::NONE;
    for (size_t c = 0; c < 10; c++){
        current_region = detect_selected_region(env, console);
        if (current_region == region){
            break;
        }
        pbf_press_dpad(console, direction, 20, 40);
        console.botbase().wait_for_all_requests();
    }
    if (current_region != region){
        console.log(std::string("Unable to find ") + MAP_REGION_NAMES[(int)region] + ".", COLOR_RED);
        return false;
    }

    if (slot != 0){
        pbf_press_button(console, BUTTON_A, 20, 105);
        for (size_t c = 0; c < slot; c++){
            pbf_press_dpad(console, DPAD_DOWN, 20, 30);
        }
    }

    //  Enter the region.
    if (!mash_A_to_change_region(env, console)){
        console.log(std::string("Unable to enter ") + MAP_REGION_NAMES[(int)region] + ".", COLOR_RED);
        return false;
    }
    return true;
}


























}
}
}
