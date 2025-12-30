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
        pbf_move_left_joystick(context, {+0.866, -1}, 240ms, 240ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, 20000ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 1000ms, 500ms);
        pbf_move_left_joystick(context, {+1, -0.252}, 240ms, 500ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        pbf_move_left_joystick(context, {-0.008, +1}, 3000ms, 1000ms);
        break;
    case LeapPokemon::Burmy:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Fieldlands_Heights,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {+0.331, -1}, 240ms, 240ms);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, 6350ms, 160ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 500ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1500ms);
        pbf_move_left_joystick(context, {+1, +0.008}, 240ms, 500ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        pbf_move_right_joystick(context, {0, -1}, 100ms, 500ms);
        break;
    case LeapPokemon::Cherrim:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Highlands_Mountain,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {+1, +0.727}, 240ms, 240ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, 3350ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 500ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1500ms);
        pbf_move_left_joystick(context, {-1, +0.008}, 160ms, 160ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        pbf_move_right_joystick(context, {0, -1}, 200ms, 500ms);
        break;
    case LeapPokemon::Cherubi:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Fieldlands_Arena,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {+1, -0.189}, 240ms, 240ms);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, 25500ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 500ms);
        break;
    case LeapPokemon::Combee:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Fieldlands_Arena,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {-0.555, -1}, 160ms, 500ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, 2750ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1000ms);
        break;
    case LeapPokemon::Heracross:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Highlands_Mountain,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {-1, -0.724}, 240ms, 240ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        pbf_move_left_joystick(context, {-0.008, +1}, 4000ms, 500ms);
        pbf_move_right_joystick(context, {0, -1}, 170ms, 1000ms);
        break;
    case LeapPokemon::Pachirisu:
    case LeapPokemon::Vespiquen:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Mirelands_Bogbound,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {-1, -0.331}, 240ms, 240ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, 10200ms, 160ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1000ms);
        break;
    case LeapPokemon::Wormadam:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Fieldlands_Fieldlands,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {-0.141, -1}, 240ms, 240ms);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, 6250ms, 160ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1000ms);
        pbf_move_left_joystick(context, {+1, +0.219}, 160ms, 160ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        pbf_move_right_joystick(context, {0, -1}, 200ms, 500ms);
        break;
    case LeapPokemon::Geodude:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Fieldlands_Heights,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {+0.567, -1}, 160ms, 160ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, 3750ms, 160ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 500ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1000ms);
        pbf_move_left_joystick(context, {0, -1}, 160ms, 160ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        break;
    case LeapPokemon::Graveler:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Highlands_Highlands,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {+1, -0.134}, 160ms, 160ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        pbf_move_left_joystick(context, {-0.008, +1}, 5000ms, 500ms);
        break;
    case LeapPokemon::Bonsly:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Mirelands_DiamondSettlement,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {-1, -0.173}, 160ms, 500ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        change_mount(stream, context, MountState::BRAVIARY_ON);
        pbf_move_left_joystick(context, {-0.008, +1}, 10000ms, 500ms);
        break;
    case LeapPokemon::Bronzor:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Highlands_Mountain,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {-0.57, -1}, 160ms, 500ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        change_mount(stream, context, MountState::WYRDEER_ON);
        pbf_press_button(context, BUTTON_B, 6500ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 500ms);
        pbf_move_right_joystick(context, {0, -1}, 200ms, 500ms);
        break;
    case LeapPokemon::Nosepass:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Highlands_Summit,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {+1, -0.37}, 160ms, 500ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        change_mount(stream, context, MountState::WYRDEER_ON);
        pbf_press_button(context, BUTTON_B, 2500ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 500ms);
        pbf_move_right_joystick(context, {0, -1}, 200ms, 500ms);
        break;
    case LeapPokemon::Bergimite:
        goto_camp_from_jubilife(
            env, stream, context,
            TravelLocations::instance().Icelands_Snowfields,
            fresh_from_reset
        );
        pbf_move_left_joystick(context, {-0.102, -1}, 160ms, 500ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 500ms);
        change_mount(stream, context, MountState::WYRDEER_ON);
        pbf_press_button(context, BUTTON_B, 3200ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 160ms, 500ms);
        pbf_move_right_joystick(context, {0, -1}, 200ms, 500ms);
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
    pbf_press_button(context, BUTTON_ZR, 500ms, 160ms); //throw pokemon
    pbf_wait(context, 4500ms);
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
            pbf_wait(context, 500ms);
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
//    pbf_press_button(context, BUTTON_B, 160ms, 1800ms);

    if (exit_method == ExitBattleMethod::RunAway){
        stream.log("Running from battle...");
        pbf_press_button(context, BUTTON_B, 160ms, 1800ms);
        pbf_press_button(context, BUTTON_A, 160ms, 4300ms);
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
