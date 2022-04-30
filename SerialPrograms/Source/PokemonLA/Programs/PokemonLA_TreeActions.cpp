/*  Tree Actions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA_RegionNavigation.h"
#include "PokemonLA_TreeActions.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"
#include "PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


void setup(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    goto_camp_from_jubilife(env, console, context, TravelLocations::instance().Fieldlands_Heights);
    pbf_move_left_joystick(context, 170, 255, 30, 30);
    change_mount(console, context, MountState::BRAVIARY_ON);
    pbf_press_button(context, BUTTON_B, (6.35 * TICKS_PER_SECOND), 20);
    pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_PLUS, 20, (0.5 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_PLUS, 20, (1.5 * TICKS_PER_SECOND));
    pbf_move_left_joystick(context, 255, 127, 30, (0.5 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
    pbf_move_right_joystick(context, 127, 255, (0.10 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
    context.wait_for_all_requests();
}

bool check_tree_for_battle(ConsoleHandle& console, BotBaseContext& context){

    pbf_press_button(context, BUTTON_ZR, (0.5 * TICKS_PER_SECOND), 20); //throw pokemon
    pbf_wait(context, (4.5 * TICKS_PER_SECOND));

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

void exit_battle(BotBaseContext& context){
    pbf_press_button(context, BUTTON_B, 20, 100);
    pbf_wait(context, (1 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_B, 20, 100);
    pbf_wait(context, (1 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_A, 20, 100);
    pbf_wait(context, (3 * TICKS_PER_SECOND));
    context.wait_for_all_requests();
}

PokemonDetails get_pokemon_details(ConsoleHandle& console, BotBaseContext& context, Language language){
    //Open Info Screen
    pbf_wait(context, (1 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_PLUS, 20, 20);
    pbf_wait(context, (1 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_R, 20, 20);
    pbf_wait(context, (1 * TICKS_PER_SECOND));

    context.wait_for_all_requests();

    QImage infoScreen = console.video().snapshot();

    StatusInfoScreenDetector detector;

    detector.process_frame(infoScreen, std::chrono::system_clock::now());
    detector.get_pokemon_name(console, infoScreen, language);

    return detector.details();;
}





















}
}
}
