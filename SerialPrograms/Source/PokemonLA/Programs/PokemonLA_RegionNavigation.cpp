/*  Region Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/Inference/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Programs/PokemonLA_EscapeFromAttack.h"
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
        throw InternalProgramError(
            &console.logger(), PA_CURRENT_FUNCTION,
            "Unknown Camp: " + std::to_string((int)camp)
        );
    }
}
void from_professor_return_to_jubilife(ProgramEnvironment& env, ConsoleHandle& console){
    ButtonDetector button_detector0(
        console, console,
        ButtonType::ButtonA, ImageFloatBox(0.500, 0.578, 0.300, 0.043),
        std::chrono::milliseconds(200), true
    );
    ButtonDetector button_detector1(
        console, console,
        ButtonType::ButtonA, ImageFloatBox(0.500, 0.621, 0.300, 0.043),
        std::chrono::milliseconds(200), true
    );
    ButtonDetector bottom_B(
        console, console,
        ButtonType::ButtonB, ImageFloatBox(0.900, 0.955, 0.080, 0.045),
        std::chrono::milliseconds(100), true
    );
    while (true){
        console.botbase().wait_for_all_requests();
        int ret = run_until(
            env, console,
            [](const BotBaseContext& context){
                for (size_t c = 0; c < 20; c++){
                    pbf_press_button(context, BUTTON_A, 20, 125);
                }
            },
            { &button_detector0, &button_detector1, &bottom_B }
        );
        env.wait_for(std::chrono::milliseconds(500));
        switch (ret){
        case 0:
            console.log("Detected return option...");
            pbf_press_dpad(console, DPAD_DOWN, 20, 105);
            mash_A_to_change_region(env, console);
            return;
        case 1:
            console.log("Detected report research option...");
            break;
        case 2:
            console.log("Backing out of Pokedex...");
            pbf_mash_button(console, BUTTON_B, 20);
            break;
        default:
            throw OperationFailedException(console, "Did not detect option to return to Jubilife.");
        }
    }
}


void mash_A_to_enter_sub_area(ProgramEnvironment& env, ConsoleHandle& console){
    BlackScreenOverWatcher black_screen0(COLOR_RED, {0.2, 0.2, 0.6, 0.6}, 100, 10);
    int ret = run_until(
        env, console,
        [](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 7 * TICKS_PER_SECOND);
        },
        { &black_screen0 }
    );
    if (ret < 0){
        throw OperationFailedException(console, "Failed to load into sub area after 7 seconds.");
    }

    console.log("Loaded into sub area...");
    env.wait_for(std::chrono::milliseconds(200));
}


void mash_A_to_change_region(ProgramEnvironment& env, ConsoleHandle& console){
    BlackScreenOverWatcher black_screen0;
    int ret = run_until(
        env, console,
        [](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 20 * TICKS_PER_SECOND);
        },
        { &black_screen0 }
    );
    if (ret < 0){
        throw OperationFailedException(console, "Failed to load into region after 20 seconds.");
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
        throw OperationFailedException(console, "Failed to load into region after 20 seconds.");
    }
    console.log("Loaded into map...");
    env.wait_for(std::chrono::seconds(1));
}


void goto_camp_from_jubilife(ProgramEnvironment& env, ConsoleHandle& console, Camp camp){
    //  Open the map.
    console.botbase().wait_for_all_requests();
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
            throw OperationFailedException(console, "Map not detected after 10 x A presses.");
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
        throw InternalProgramError(
            &console.logger(), PA_CURRENT_FUNCTION,
            "Invalid Camp Enum: " + std::to_string((int)camp)
        );
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
        throw OperationFailedException(console, std::string("Unable to find ") + MAP_REGION_NAMES[(int)region] + ".");
    }

    if (slot != 0){
        pbf_press_button(console, BUTTON_A, 20, 105);
        for (size_t c = 0; c < slot; c++){
            pbf_press_dpad(console, DPAD_DOWN, 20, 30);
        }
    }

    //  Enter the region.
    mash_A_to_change_region(env, console);
}




void goto_camp_from_overworld(
    ProgramEnvironment& env, ConsoleHandle& console,
    ShinyDetectedActionOption& options,
    ShinyStatIncrementer& shiny_stat_incrementer
){
    auto start = std::chrono::system_clock::now();
    std::chrono::seconds grace_period(0);
    while (true){
        EscapeFromAttack session(
            env, console,
            grace_period, std::chrono::seconds(10),
            options.stop_on_shiny()
        );
        session.run_session();

        if (session.detected_shiny()){
            shiny_stat_incrementer.add_shiny();
            on_shiny_sound(env, console, options, session.shiny_sound_results());
        }

        if (std::chrono::system_clock::now() - start > std::chrono::seconds(60)){
            throw OperationFailedException(console, "Unable to escape from being attacked.");
        }

        //  Open the map.
        pbf_press_button(console, BUTTON_MINUS, 20, 30);
        {
            MapDetector detector;
            int ret = wait_until(
                env, console,
                std::chrono::seconds(5),
                { &detector }
            );
            if (ret < 0){
                console.log("Map not detected after 5 seconds.", COLOR_RED);
                pbf_mash_button(console, BUTTON_B, TICKS_PER_SECOND);
                console.botbase().wait_for_all_requests();
                continue;
            }
            console.log("Found map!");
            env.wait_for(std::chrono::milliseconds(500));
        }

        //  Try to fly back to camp.
        pbf_press_button(console, BUTTON_X, 20, 30);

        {
            ButtonDetector detector(
                console, console,
                ButtonType::ButtonA,
                {0.55, 0.40, 0.20, 0.40},
                std::chrono::milliseconds(200), true
            );
            int ret = wait_until(
                env, console,
                std::chrono::seconds(2),
                { &detector }
            );
            if (ret >= 0){
                console.log("Flying back to camp...");
                pbf_mash_button(console, BUTTON_A, 125);
                break;
            }
            console.log("Unable to fly. Are you under attack?", COLOR_RED);
        }

        pbf_mash_button(console, BUTTON_B, 125);
        grace_period = std::chrono::seconds(5);
    }

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.6});
    int ret = wait_until(
        env, console,
        std::chrono::seconds(20),
        { &black_screen }
    );
    if (ret < 0){
        throw OperationFailedException(console, "Failed to fly to camp after 20 seconds.");
    }
    console.log("Arrived at camp...");
    env.wait_for(std::chrono::seconds(1));
}






















}
}
}
