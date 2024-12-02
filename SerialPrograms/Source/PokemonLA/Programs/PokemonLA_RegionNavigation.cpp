/*  Region Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Programs/PokemonLA_EscapeFromAttack.h"
#include "PokemonLA_RegionNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



void goto_professor(Logger& logger, BotBaseContext& context, Camp camp){
    switch (camp){
    case Camp::FIELDLANDS_FIELDLANDS:
        pbf_move_left_joystick(context, 255, 0, 125, 0);
        return;
    case Camp::FIELDLANDS_HEIGHTS:
        pbf_move_left_joystick(context, 240, 0, 200, 0);
        return;
    case Camp::MIRELANDS_MIRELANDS:
        pbf_move_left_joystick(context, 255, 64, 160, 0);
        return;
    case Camp::MIRELANDS_BOGBOUND:
        pbf_move_left_joystick(context, 255, 64, 140, 0);
        return;
    case Camp::COASTLANDS_BEACHSIDE:
        pbf_move_left_joystick(context, 255, 96, 125, 0);
        return;
    case Camp::COASTLANDS_COASTLANDS:
        pbf_move_left_joystick(context, 255, 48, 105, 0);
        return;
    case Camp::HIGHLANDS_HIGHLANDS:
        pbf_move_left_joystick(context, 255, 64, 176, 0);
        return;
    case Camp::HIGHLANDS_MOUNTAIN:
        pbf_move_left_joystick(context, 255, 32, 125, 0);
        return;
    case Camp::HIGHLANDS_SUMMIT:
        pbf_move_left_joystick(context, 255, 0, 125, 0);
        return;
    case Camp::ICELANDS_SNOWFIELDS:
        pbf_move_left_joystick(context, 255, 56, 125, 0);
        return;
    case Camp::ICELANDS_ICEPEAK:
        pbf_move_left_joystick(context, 255, 48, 75, 0);
        return;
    default:
        throw InternalProgramError(
            &logger, PA_CURRENT_FUNCTION,
            "Unknown Camp: " + std::to_string((int)camp)
        );
    }
}
void from_professor_return_to_jubilife(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
){
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
        context.wait_for_all_requests();
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                for (size_t c = 0; c < 20; c++){
                    pbf_press_button(context, BUTTON_A, 20, 125);
                }
            },
            {
                {button_detector0},
                {button_detector1},
                {bottom_B}
            }
        );
        context.wait_for(std::chrono::milliseconds(500));
        switch (ret){
        case 0:
            console.log("Detected return option...");
            pbf_press_dpad(context, DPAD_DOWN, 20, 105);
            mash_A_to_change_region(env, console, context);
            return;
        case 1:
            console.log("Detected report research option...");
            break;
        case 2:
            console.log("Backing out of Pokedex...");
            pbf_mash_button(context, BUTTON_B, 20);
            break;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "Did not detect option to return to Jubilife.",
                true
            );
        }
    }
}


void mash_A_to_enter_sub_area(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
){
    BlackScreenOverWatcher black_screen0(COLOR_RED, {0.2, 0.2, 0.6, 0.6}, 100, 10);
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 7 * TICKS_PER_SECOND);
        },
        {{black_screen0}}
    );
    if (ret < 0){
//        dump_image(env.logger(), env.program_info(), "LoadSubArea", console.video().snapshot());
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to load into sub area after 7 seconds.",
            true
        );
    }

    console.log("Loaded into sub area...");
    context.wait_for(std::chrono::milliseconds(200));
}


void mash_A_to_change_region(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
){
    BlackScreenOverWatcher black_screen0;
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, GameSettings::instance().LOAD_REGION_TIMEOUT);
        },
        {{black_screen0}}
    );
    if (ret < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to load into region after timeout.",
            true
        );
    }
    context.wait_for(std::chrono::milliseconds(1000));

    BlackScreenOverWatcher black_screen1a(COLOR_RED, {0.20, 0.02, 0.60, 0.05}, 150);
    BlackScreenOverWatcher black_screen1b(COLOR_RED, {0.20, 0.93, 0.60, 0.05}, 150);
    ret = wait_until(
        console, context,
        std::chrono::milliseconds(1000 * GameSettings::instance().LOAD_REGION_TIMEOUT / TICKS_PER_SECOND),
        {
            {black_screen1a},
            {black_screen1b},
        }
    );
    if (ret < 0){
//        dump_image(console.logger(), env.program_info(), "LoadRegion", console.video().snapshot());
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to load into region after timeout.",
            true
        );
    }
    console.log("Loaded into map...");
    context.wait_for(std::chrono::milliseconds((uint64_t)(GameSettings::instance().POST_WARP_DELAY * 1000)));
}


void open_travel_map_from_jubilife(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
){
    pbf_move_left_joystick(context, 128, 255, 200, 0);
    MapDetector detector;
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, BUTTON_A, 20, 105);
            }
        },
        {{detector}}
    );
    if (ret < 0){
//        dump_image(console.logger(), env.program_info(), "MapNotDetected", console.video().snapshot());
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Map not detected after 10 x A presses.",
            true
        );
    }
    console.log("Found map!");
}


void goto_camp_from_jubilife(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    const TravelLocation& location
){
    // Move backwards and talk to guard to open the map.
    context.wait_for_all_requests();
    open_travel_map_from_jubilife(env, console, context);
    context.wait_for(std::chrono::milliseconds(500));

    DpadPosition direction = location.region < MapRegion::HIGHLANDS ? DPAD_RIGHT : DPAD_LEFT;

    //  Move to region.
    MapRegion current_region = MapRegion::NONE;
    for (size_t c = 0; c < 10; c++){
        current_region = detect_selected_region(console, context);
        if (current_region == location.region){
            break;
        }
        pbf_press_dpad(context, direction, 20, 40);
        context.wait_for_all_requests();
    }
    if (current_region != location.region){
//        dump_image(console.logger(), env.program_info(), "FindRegion", console.video().snapshot());
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            std::string("Unable to find: ") + location.display,
            true
        );
    }

    if (location.warp_slot != 0){
        pbf_press_button(context, BUTTON_A, 20, 105);
        for (size_t c = 0; c < location.warp_slot; c++){
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
        }
    }

    //  Enter the region.
    mash_A_to_change_region(env, console, context);

    if (location.warp_sub_slot == 0){
        // The destination is a camp.
        // No need to do a warp in the region.
        return;
    }

    // The destination is not a camp.
    // It's a settlement or arena that requires another warp:

    //  Open the map.
    pbf_press_button(context, BUTTON_MINUS, 20, 30);
    {
        MapDetector detector;
        int ret = wait_until(
            console, context,
            std::chrono::seconds(5),
            {{detector}}
        );
        if (ret < 0){
//            auto snapshot = console.video().snapshot();
//            dump_image(env.logger(), env.program_info(), "MapNotFound", snapshot);
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "Map not detected after 5 seconds.",
                true
            );
        }
        console.log("Found map!");
        context.wait_for(std::chrono::milliseconds(500));
    }

    //  Warp to sub-camp.
    pbf_press_button(context, BUTTON_X, 20, 30);
    {
        ButtonDetector detector(
            console, console,
            ButtonType::ButtonA,
            {0.55, 0.40, 0.20, 0.40},
            std::chrono::milliseconds(200), true
        );
        int ret = wait_until(
            console, context,
            std::chrono::seconds(2),
            {{detector}}
        );
        if (ret < 0){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "Unable to fly. Are you under attack?",
                true
            );
        }
    }
    pbf_wait(context, 50);
    for (size_t c = 0; c < location.warp_sub_slot; c++){
        const DpadPosition dir = (location.reverse_sub_menu_direction ? DPAD_UP : DPAD_DOWN);
        pbf_press_dpad(context, dir, 20, 30);
    }
    pbf_mash_button(context, BUTTON_A, 125);

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.6});
    int ret = wait_until(
        console, context,
        std::chrono::seconds(20),
        {{black_screen}}
    );
    if (ret < 0){
//        dump_image(console.logger(), env.program_info(), "FlyToCamp", console.video().snapshot());
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to fly to camp after 20 seconds.",
            true
        );
    }
    console.log("Arrived at sub-camp...");
    context.wait_for(std::chrono::milliseconds((uint64_t)(GameSettings::instance().POST_WARP_DELAY * 1000)));

    if (location.post_arrival_maneuver == nullptr){
        return;
    }

    location.post_arrival_maneuver(console, context);
    context.wait_for_all_requests();
}




void goto_camp_from_overworld(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
){
    auto start = current_time();
    std::chrono::seconds grace_period(0);
    while (true){
        {
            EscapeFromAttack session(
                env, console, context,
                grace_period, std::chrono::seconds(10)
            );
            session.run_session();
        }

        if (current_time() - start > std::chrono::seconds(60)){
//            dump_image(console.logger(), env.program_info(), "EscapeFromAttack", console.video().snapshot());
            throw OperationFailedException(
                ErrorReport::NO_ERROR_REPORT, console,
                "Unable to escape from being attacked.",
                true
            );
        }

        //  Open the map.
        pbf_press_button(context, BUTTON_MINUS, 20, 30);
        {
            MapDetector detector;
            int ret = wait_until(
                console, context,
                std::chrono::seconds(5),
                {{detector}}
            );
            if (ret < 0){
//                dump_image(console.logger(), env.program_info(), "MapNotDetected", console.video().snapshot());
                console.log("Map not detected after 5 seconds.", COLOR_RED);
                pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
                context.wait_for_all_requests();
                continue;
            }
            console.log("Found map!");
            context.wait_for(std::chrono::milliseconds(500));
        }

        //  Try to fly back to camp.
        pbf_press_button(context, BUTTON_X, 20, 30);

        {
            ButtonDetector detector(
                console, console,
                ButtonType::ButtonA,
                {0.55, 0.40, 0.20, 0.40},
                std::chrono::milliseconds(200), true
            );
            int ret = wait_until(
                console, context,
                std::chrono::seconds(2),
                {{detector}}
            );
            if (ret >= 0){
                console.log("Flying back to camp...");
                pbf_mash_button(context, BUTTON_A, 125);
                break;
            }
            console.log("Unable to fly. Are you under attack?", COLOR_RED);
        }

        pbf_mash_button(context, BUTTON_B, 125);
        grace_period = std::chrono::seconds(5);
    }

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.6});
    int ret = wait_until(
        console, context,
        std::chrono::seconds(20),
        {{black_screen}}
    );
    if (ret < 0){
//        auto snapshot = console.video().snapshot();
//        dump_image(console.logger(), env.program_info(), "FlyToCamp", snapshot);
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to fly to camp after 20 seconds.",
            true
        );
    }
    console.log("Arrived at camp...");
    context.wait_for(std::chrono::milliseconds((uint64_t)(GameSettings::instance().POST_WARP_DELAY * 1000)));
}

void goto_any_camp_from_overworld(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    const TravelLocation& location
){

    auto start = current_time();
    std::chrono::seconds grace_period(0);
    while (true){
        {
            EscapeFromAttack session(
                env, console, context,
                grace_period, std::chrono::seconds(10)
            );
            session.run_session();
        }

        if (current_time() - start > std::chrono::seconds(60)){
//            dump_image(console.logger(), env.program_info(), "EscapeFromAttack", console.video().snapshot());
            throw OperationFailedException(
                ErrorReport::NO_ERROR_REPORT, console,
                "Unable to escape from being attacked.",
                true
            );
        }

        //  Open the map.
        pbf_press_button(context, BUTTON_MINUS, 20, 30);
        {
            MapDetector detector;
            int ret = wait_until(
                console, context,
                std::chrono::seconds(5),
                {{detector}}
            );
            if (ret < 0){
//                dump_image(env.logger(), env.program_info(), "MapNotFound", console.video().snapshot());
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "Map not detected after 5 seconds.",
                    true
                );
            }
            console.log("Found map!");
            context.wait_for(std::chrono::milliseconds(500));
        }

        //  Warp to sub-camp.
        pbf_press_button(context, BUTTON_X, 20, 30);
        {
            ButtonDetector detector(
                console, console,
                ButtonType::ButtonA,
                {0.55, 0.40, 0.20, 0.40},
                std::chrono::milliseconds(200), true
            );
            int ret = wait_until(
                console, context,
                std::chrono::seconds(2),
                {{detector}}
            );
            if (ret >= 0){
                console.log("Flying back to camp...");
                pbf_wait(context, 50);
                if (location.warp_slot != 0){
                    for (size_t c = 0; c < location.warp_slot; c++){
                        pbf_press_dpad(context, DPAD_DOWN, 20, 30);
                    }
                }
                for (size_t c = 0; c < location.warp_sub_slot; c++){
                    const DpadPosition dir = (location.reverse_sub_menu_direction ? DPAD_UP : DPAD_DOWN);
                    pbf_press_dpad(context, dir, 20, 30);
                }
                pbf_mash_button(context, BUTTON_A, 125);
                break;
            }
        }

        pbf_mash_button(context, BUTTON_B, 125);
        grace_period = std::chrono::seconds(5);
    }
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.6});
    int ret = wait_until(
        console, context,
        std::chrono::seconds(20),
        {{black_screen}}
    );
    if (ret < 0){
//        dump_image(console.logger(), env.program_info(), "FlyToCamp", console.video().snapshot());
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to fly to camp after 20 seconds.",
            true
        );
    }
    console.log("Arrived at camp...");
    context.wait_for(std::chrono::milliseconds((uint64_t)(GameSettings::instance().POST_WARP_DELAY * 1000)));
    console.botbase().wait_for_all_requests();
}


void goto_Mai_from_camp(
    Logger& logger, BotBaseContext& context, Camp camp
){
    switch (camp){
    case Camp::FIELDLANDS_FIELDLANDS:
        // 80 - 128, time - 400
        pbf_move_left_joystick(context, 85, 255, 300, 0);
        return;
    case Camp::MIRELANDS_MIRELANDS:
        pbf_move_left_joystick(context, 0, 120, 310, 0);
        return;
    case Camp::COASTLANDS_BEACHSIDE:
        // 255, 150 -170, 600 too long
        pbf_move_left_joystick(context, 255, 165, 550, 0);
        return;
    case Camp::HIGHLANDS_HIGHLANDS:
        // 255, 150 - 170
        pbf_move_left_joystick(context, 255, 165, 370, 0);
        return;
    case Camp::ICELANDS_SNOWFIELDS:
        pbf_move_left_joystick(context, 255, 124, 250, 0);
        return;
    default:
        throw InternalProgramError(
            &logger, PA_CURRENT_FUNCTION,
            "Unknown Camp when going to Mai: " + std::to_string((int)camp)
        );
    }
}

void goto_professor(Logger& logger, BotBaseContext& context, const TravelLocation& location){
    Camp camp = map_region_default_camp(location.region);
    goto_professor(logger, context, camp);
}


















}
}
}
