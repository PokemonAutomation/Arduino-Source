/*  Tree Actions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


void BurmyPaths(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    //Tree 1
    goto_camp_from_jubilife(env, console, context, TravelLocations::instance().Fieldlands_Heights);
    pbf_move_left_joystick(context, 170, 255, 30, 30);
    change_mount(console, context, MountState::BRAVIARY_ON);
    pbf_press_button(context, BUTTON_B, (6.35 * TICKS_PER_SECOND), 20);
    pbf_press_button(context, BUTTON_PLUS, 20, 20);
    pbf_wait(context, (1 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_PLUS, 20, 20);
    pbf_wait(context, (0.5 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_PLUS, 20, 20);
    pbf_wait(context, (1 * TICKS_PER_SECOND));
    pbf_move_left_joystick(context, 255, 127, 30, 30);
    pbf_wait(context, (0.5 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_ZL, 20, 20);
    pbf_wait(context, (0.5 * TICKS_PER_SECOND));
    pbf_move_right_joystick(context, 127, 255, (0.10 * TICKS_PER_SECOND), 20);
    pbf_wait(context, (0.5 * TICKS_PER_SECOND));
    context.wait_for_all_requests();

    //Tree 2
//    goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
//    pbf_move_left_joystick(context, 152, 255, 30, 30);
//    change_mount(console, context, MountState::BRAVIARY_ON);
//    pbf_press_button(context, BUTTON_B, (11.8 * TICKS_PER_SECOND), 20);
//    pbf_press_button(context, BUTTON_PLUS, 20, 20);
//    pbf_wait(context, (1 * TICKS_PER_SECOND));
//    pbf_press_button(context, BUTTON_PLUS, 20, 20);
//    pbf_wait(context, (0.5 * TICKS_PER_SECOND));
//    pbf_press_button(context, BUTTON_PLUS, 20, 20);
//    pbf_wait(context, (1 * TICKS_PER_SECOND));
//    pbf_move_left_joystick(context, 255, 127, 30, 30);
//    pbf_wait(context, (0.5 * TICKS_PER_SECOND));
//    pbf_press_button(context, BUTTON_ZL, 20, 20);
//    pbf_wait(context, (0.5 * TICKS_PER_SECOND));
//    pbf_move_right_joystick(context, 127, 255, (0.10 * TICKS_PER_SECOND), 20);
//    pbf_wait(context, (0.5 * TICKS_PER_SECOND));
//    context.wait_for_all_requests();
}

bool check_tree(ConsoleHandle& console, BotBaseContext& context){

    pbf_press_button(context, BUTTON_ZR, (0.5 * TICKS_PER_SECOND), 20); //throw pokemon
    pbf_wait(context, (4 * TICKS_PER_SECOND));
    context.wait_for_all_requests();

    MountDetector mount_detector;
    MountState mount = mount_detector.detect(console.video().snapshot());

    if (mount != MountState::NOTHING){
       console.log("Battle not found. Tree might be empty.");
       return false;
    }

    console.log("Battle found!");

    BattleMenuDetector battle_menu_detector(console, console, true);
    wait_until(
       console, context, std::chrono::seconds(10),
       {
           {battle_menu_detector}
       }
    );

    return true;
}





















}
}
}
