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

    BlackScreenOverWatcher black_screen1(COLOR_RED, {0.20, 0.95, 0.60, 0.03}, 20);
    ret = wait_until(
        env, console,
        std::chrono::seconds(20),
        { &black_screen1 }
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



}
}
}
