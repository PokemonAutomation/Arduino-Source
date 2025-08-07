/*  Leap Pokemon Actions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"
#include "PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA_MountChange.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA_RegionNavigation.h"
#include "PokemonLA_BattleRoutines.h"
#include "PokemonLA_LeapPokemonActions.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

void route(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    LeapPokemon pokemon,
    bool fresh_from_reset
){
    switch (pokemon){
    case LeapPokemon::Aipom:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Coastlands_Beachside,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 238, 255, 30, 30);
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (20 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, (1 * TICKS_PER_SECOND), (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_left_joystick(context, 255, 160, 30, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_left_joystick(context, 127, 0, (3 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Burmy:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Fieldlands_Heights,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 170, 255, 30, 30);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (uint16_t)(6.35 * TICKS_PER_SECOND), 20);
        pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (uint16_t)(1.5 * TICKS_PER_SECOND));
        pbf_move_left_joystick(context, 255, 127, 30, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_right_joystick(context, 127, 255, (uint16_t)(0.10 * TICKS_PER_SECOND), (uint16_t)(0.5 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Cherrim:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Highlands_Mountain,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 255, 35, 30, 30);
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (uint16_t)(3.35 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (uint16_t)(1.5 * TICKS_PER_SECOND));
        pbf_move_left_joystick(context, 0, 127, 20, 20);
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_right_joystick(context, 127, 255, (uint16_t)(0.20 * TICKS_PER_SECOND), (uint16_t)(0.5 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Cherubi:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Fieldlands_Arena,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 255, 152, 30, 30);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (uint16_t)(25.5 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Combee:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Fieldlands_Arena,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 57, 255, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (uint16_t)(2.75 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Heracross:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Highlands_Mountain,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 0, 220, 30, 30);
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_left_joystick(context, 127, 0, (4 * TICKS_PER_SECOND), (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_right_joystick(context, 127, 255, (uint16_t)(0.17 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Pachirisu:
    case LeapPokemon::Vespiquen:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Mirelands_Bogbound,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 0, 170, 30, 30);
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (uint16_t)(10.2 * TICKS_PER_SECOND), 20);
        pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Wormadam:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Fieldlands_Fieldlands,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 110, 255, 30, 30);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (uint16_t)(6.25 * TICKS_PER_SECOND), 20);
        pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
        pbf_move_left_joystick(context, 255, 100, 20, 20);
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_right_joystick(context, 127, 255, (uint16_t)(0.2 * TICKS_PER_SECOND), (uint16_t)(0.5 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Geodude:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Fieldlands_Heights,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 200, 255, 20, 20);
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (uint16_t)(3.75 * TICKS_PER_SECOND), 20);
        pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
        pbf_move_left_joystick(context, 127, 255, 20, 20);
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Graveler:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Highlands_Highlands,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 255, 145, 20, 20);
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_left_joystick(context, 127, 0, (5 * TICKS_PER_SECOND), (uint16_t)(0.5 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Bonsly:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Mirelands_DiamondSettlement,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 0, 150, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_move_left_joystick(context, 127, 0, (10 * TICKS_PER_SECOND), (uint16_t)(0.5 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Bronzor:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Highlands_Mountain,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 55, 255, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        change_mount(stream, context, MountState::WYRDEER_ON);
        pbf_press_button(context, BUTTON_B, (uint16_t)(6.5 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_right_joystick(context, 127, 255, (uint16_t)(0.2 * TICKS_PER_SECOND), (uint16_t)(0.5 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Nosepass:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Highlands_Summit,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 255, 175, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        change_mount(stream, context, MountState::WYRDEER_ON);
        pbf_press_button(context, BUTTON_B, (uint16_t)(2.5 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_right_joystick(context, 127, 255, (uint16_t)(0.2 * TICKS_PER_SECOND), (uint16_t)(0.5 * TICKS_PER_SECOND));
        break;
    case LeapPokemon::Bergimite:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Icelands_Snowfields,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, 115, 255, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_ZL, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        change_mount(stream, context, MountState::WYRDEER_ON);
        pbf_press_button(context, BUTTON_B, (uint16_t)(3.2 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, (uint16_t)(0.5 * TICKS_PER_SECOND));
        pbf_move_right_joystick(context, 127, 255, (uint16_t)(0.2 * TICKS_PER_SECOND), (uint16_t)(0.5 * TICKS_PER_SECOND));
        break;
    }
    context.wait_for_all_requests();
}

void return_to_jubilife(ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context, LeapPokemon pokemon){
    goto_camp_from_overworld(env, stream, context);
    switch (pokemon){
    case LeapPokemon::Aipom:
        goto_professor(stream.logger(), context, Camp::COASTLANDS_BEACHSIDE);
        break;
    case LeapPokemon::Burmy:
    case LeapPokemon::Cherubi:
    case LeapPokemon::Combee:
    case LeapPokemon::Wormadam:
    case LeapPokemon::Geodude:
        goto_professor(stream.logger(), context, Camp::FIELDLANDS_FIELDLANDS);
        break;
    case LeapPokemon::Cherrim:
    case LeapPokemon::Heracross:
    case LeapPokemon::Graveler:
    case LeapPokemon::Bronzor:
    case LeapPokemon::Nosepass:
        goto_professor(stream.logger(), context, Camp::HIGHLANDS_HIGHLANDS);
        break;
    case LeapPokemon::Pachirisu:
    case LeapPokemon::Vespiquen:
    case LeapPokemon::Bonsly:
        goto_professor(stream.logger(), context, Camp::MIRELANDS_MIRELANDS);
        break;
    case LeapPokemon::Bergimite:
        goto_professor(stream.logger(), context, Camp::ICELANDS_SNOWFIELDS);
        break;
    }
    from_professor_return_to_jubilife(env, stream, context);
    context.wait_for_all_requests();
}

bool check_tree_or_ore_for_battle(VideoStream& stream, ProControllerContext& context){
    pbf_press_button(context, BUTTON_ZR, (uint16_t)(0.5 * TICKS_PER_SECOND), 20); //throw pokemon
    pbf_wait(context, (uint16_t)(4.5 * TICKS_PER_SECOND));
    context.wait_for_all_requests();

    MountDetector mount_detector;

    VideoSnapshot snapshot = stream.video().snapshot();
    MountState mount = mount_detector.detect(snapshot);
    bool found = false;
    for (int i = 0; i < 4; i++){
        if (mount == MountState::NOTHING){
            found = true;
            break;
        }else{
            stream.log("Attempt " + std::to_string(i) + ". Found: " + MOUNT_STATE_STRINGS[(int)mount]);
            pbf_wait(context, (uint16_t)(0.5 * TICKS_PER_SECOND));
            context.wait_for_all_requests();
            snapshot = stream.video().snapshot();
        }
        mount = mount_detector.detect(snapshot);
    }

    if (!found){
        stream.log("Battle not found. Tree or ore might be empty.");
        return false;
    }

    stream.log("Mount icon seems to be gone, waiting for battle menu...");

    BattleMenuDetector battle_menu_detector(stream.logger(), stream.overlay(), true);
    wait_until(
       stream, context, std::chrono::seconds(10),
       {{battle_menu_detector}}
    );

    if(battle_menu_detector.detected()){
        stream.log("Battle found!");
        return true;
    }

    stream.log("Battle menu not found. Tree or ore might be empty.");
    return false;
}

void exit_battle(VideoStream& stream, ProControllerContext& context, ExitBattleMethod exit_method){
//    pbf_press_button(context, BUTTON_B, 20, 225);

    if (exit_method == ExitBattleMethod::RunAway){
        stream.log("Running from battle...");
        pbf_press_button(context, BUTTON_B, 20, 225);
        pbf_press_button(context, BUTTON_A, 20, 100 + (uint16_t)(3.5 * TICKS_PER_SECOND));
        context.wait_for_all_requests();
        return;
    }

    context.wait_for_all_requests();
    stream.log("Mashing A to battle!");
    mash_A_until_end_of_battle(stream, context);
}

PokemonDetails get_pokemon_details(VideoStream& stream, ProControllerContext& context, Language language){
    //  Open Info Screen
    pbf_wait(context, 1000ms);
    pbf_press_button(context, BUTTON_PLUS, 500ms, 1000ms);
    pbf_press_button(context, BUTTON_R, 500ms, 1000ms);

    context.wait_for_all_requests();
    VideoSnapshot screen = stream.video().snapshot();

    return read_status_info(stream.logger(), stream.overlay(), screen, language);
}





















}
}
}
