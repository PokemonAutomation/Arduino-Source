/*  Blueberry Catch Photo
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h"
#include "PokemonSV/Programs/PokemonSV_Terarium.h"
#include "PokemonSV_BlueberryQuests.h"

#include<vector>
#include "PokemonSV_BlueberryCatchPhoto.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

CameraAngle quest_photo_navi(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
){
    CameraAngle angle = CameraAngle::none;

    //Navigate to target
    switch (current_quest){
        case BBQuests::photo_fly: case BBQuests::photo_psychic:
            stream.log("Photo: In-flight/Psychic");

            //Polar Rest Area - targeting Duosion fixed spawn
            central_to_polar_rest(info, stream, context);

            pbf_press_button(context, BUTTON_L, 10, 50);
            pbf_move_left_joystick(context, 128, 0, 230, 20);
            pbf_move_left_joystick(context, 0, 128, 250, 20);

            break;
        case BBQuests::photo_swim: case BBQuests::photo_water: case BBQuests::photo_polar:
            stream.log("Photo: Swimming/Water/Polar");

            //Polar Outdoor Classroom 1 - fixed Horsea
            central_to_polar_class1(info, stream, context);

            pbf_wait(context, 200);
            context.wait_for_all_requests();

            pbf_press_button(context, BUTTON_L, 10, 50);
            pbf_move_left_joystick(context, 255, 50, 180, 20);

            angle = CameraAngle::down;

            break;
        case BBQuests::photo_coastal: case BBQuests::photo_grass: case BBQuests::photo_dragon:
            stream.log("Photo: Coastal/Grass/Dragon");

            //Coastal Plaza - Exeggutor-A
            central_to_coastal_plaza(info, stream, context);

            pbf_move_left_joystick(context, 0, 115, 400, 20);

            //Jump down
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);
            ssf_press_button(context, BUTTON_B, 0, 100);
            ssf_press_button(context, BUTTON_B, 0, 20, 10);
            ssf_press_button(context, BUTTON_B, 0, 20);

            pbf_wait(context, 100);
            context.wait_for_all_requests();

            pbf_move_left_joystick(context, 128, 0, 150, 20);
            pbf_press_button(context, BUTTON_B, 20, 20);
            pbf_wait(context, 200);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);
            context.wait_for_all_requests();

            break;
        case BBQuests::photo_canyon: case BBQuests::photo_ghost: case BBQuests::photo_ground:
            stream.log("Photo: Canyon/Ghost/Ground");

            //Canyon Plaza - Golett
            central_to_canyon_plaza(info, stream, context);

            pbf_move_left_joystick(context, 210, 128, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 600, 250, 400);

            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            break;
        case BBQuests::photo_savanna: case BBQuests::photo_normal: case BBQuests::photo_fire:
            stream.log("Photo: Normal/Fire");

            //Savanna Plaza - Pride Rock
            central_to_savanna_plaza(info, stream, context);

            pbf_move_left_joystick(context, 220, 255, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 600, 400, 400);

            pbf_press_button(context, BUTTON_PLUS, 20, 105);
            pbf_move_left_joystick(context, 255, 128, 20, 50);

            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 128, 0, 100, 50);
            pbf_move_left_joystick(context, 0, 0, 20, 50);

            break;
        case BBQuests::photo_bug: case BBQuests::photo_rock:
            stream.log("Photo: Bug/Rock");

            //Kleavor
            central_to_canyon_plaza(info, stream, context);

            pbf_move_left_joystick(context, 205, 64, 20, 105);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 1000, 1500, 300);

            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            break;
        case BBQuests::photo_fairy:
            stream.log("Photo: Fairy");

            //Snubbull - Central plaza
            open_map_from_overworld(info, stream, context);
            fly_to_overworld_from_map(info, stream, context);

            pbf_move_left_joystick(context, 0, 80, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 2000, 1500, 200);

            pbf_press_button(context, BUTTON_PLUS, 20, 105);

            pbf_move_left_joystick(context, 255, 0, 10, 20);
            pbf_press_button(context, BUTTON_L, 20, 50);

            angle = CameraAngle::down;
            break;
        case BBQuests::photo_ice:
            stream.log("Photo: Ice");

            //Snover - Start at central plaza, fly north-ish
            open_map_from_overworld(info, stream, context);
            fly_to_overworld_from_map(info, stream, context);

            pbf_move_left_joystick(context, 0, 0, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 1100, 1700, 200);

            pbf_press_button(context, BUTTON_PLUS, 20, 105);
            pbf_move_left_joystick(context, 0, 128, 20, 50);
            pbf_press_button(context, BUTTON_L, 20, 50);

            break;
        case BBQuests::photo_flying: case BBQuests::photo_dark:
            stream.log("Photo: Dark/Flying");

            //Vullaby/Mandibuzz
            central_to_savanna_plaza(info, stream, context);

            pbf_move_left_joystick(context, 255, 0, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            pbf_move_left_joystick(context, 128, 0, 550, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            pbf_move_left_joystick(context, 128, 0, 150, 20);

            pbf_move_left_joystick(context, 255, 128, 10, 20);

            angle = CameraAngle::down;
            break;
        case BBQuests::photo_electric:
            stream.log("Photo: Electric");

            //Electabuzz
            central_to_canyon_rest(info, stream, context);

            pbf_move_left_joystick(context, 255, 255, 10, 20);
            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 128, 0, 400, 20);

            break;
        case BBQuests::photo_fighting:
            stream.log("Photo: Fighting");

            //Hitmontop (TERA ICE) - Canyon Plaza or Classroom
            central_to_canyon_plaza(info, stream, context);
            pbf_move_left_joystick(context, 0, 128, 400, 20);
            pbf_press_button(context, BUTTON_L, 10, 50);
            pbf_move_left_joystick(context, 0, 100, 20, 50);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 200, 500, 800);

            pbf_press_button(context, BUTTON_PLUS, 20, 100);
            context.wait_for_all_requests();

            break;
        case BBQuests::photo_poison:
            stream.log("Photo: Poison");

            //Muk-A - area a bit laggy but consistently so
            central_to_coastal_plaza(info, stream, context);
            pbf_move_left_joystick(context, 0, 128, 20, 50);

            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);
            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 1000, 1400, 300);

            pbf_press_button(context, BUTTON_PLUS, 20, 105);

            pbf_move_left_joystick(context, 128, 0, 150, 50);
            pbf_move_left_joystick(context, 180, 0, 20, 50);
            pbf_wait(context, 200); //Give it time to spawn/load.
            context.wait_for_all_requests();

            angle = CameraAngle::down;

            break;
        case BBQuests::photo_steel:
            stream.log("Photo: Steel");

            //Dugtrio-A - area a bit laggy but should work most of the time
            central_to_coastal_plaza(info, stream, context);
            pbf_move_left_joystick(context, 0, 128, 20, 50);

            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);
            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 200, 575, 200);

            pbf_press_button(context, BUTTON_PLUS, 20, 105);
            pbf_move_left_joystick(context, 0, 128, 20, 50);
            pbf_press_button(context, BUTTON_L, 20, 50);

            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Invalid photo quest.",
                stream
            );
            break;
    }

    return angle;
}

void quest_photo(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
){
    bool took_photo = false;
    CameraAngle move_camera = CameraAngle::none;

    while(!took_photo){
        EncounterWatcher encounter_watcher(stream, COLOR_RED);
        int ret = run_until<SwitchControllerContext>(
            stream, context,
            [&](SwitchControllerContext& context){

                move_camera = quest_photo_navi(info, stream, context, BBQ_OPTIONS, current_quest);

                //Take photo.
                stream.log("Taking photo.");
                PromptDialogWatcher photo_prompt(COLOR_RED);
                OverworldWatcher overworld(stream.logger(), COLOR_BLUE);

                pbf_press_dpad(context, DPAD_DOWN, 50, 20);
                pbf_wait(context, 100);
                context.wait_for_all_requests();

                if (move_camera == CameraAngle::up){
                    pbf_move_right_joystick(context, 128, 0, 50, 20);
                }else if (move_camera == CameraAngle::down){
                    pbf_move_right_joystick(context, 128, 255, 50, 20);
                }
                context.wait_for_all_requests();

                pbf_press_button(context, BUTTON_A, 20, 50);

                int ret = wait_until(
                    stream, context,
                    std::chrono::seconds(10),
                    { photo_prompt }
                );
                if (ret != 0){
                    stream.log("Photo not taken.");
                }

                //Mash B until overworld
                int exit = run_until<SwitchControllerContext>(
                    stream, context,
                    [&](SwitchControllerContext& context){
                        pbf_mash_button(context, BUTTON_B, 2000);
                    },
                    {{ overworld }}
                );
                if (exit == 0){
                    stream.log("Overworld detected.");
                }
                took_photo = true;
                context.wait_for_all_requests();
            },
            {
                static_cast<VisualInferenceCallback&>(encounter_watcher),
                static_cast<AudioInferenceCallback&>(encounter_watcher),
            }
        );
        encounter_watcher.throw_if_no_sound();

        if (ret >= 0){
            stream.log("Battle menu detected.");

            bool is_shiny = (bool)encounter_watcher.shiny_screenshot();
            if (is_shiny){
                stream.log("Shiny detected!");
                pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
                throw ProgramFinishedException();
            }else{
                stream.log("Detected battle. Running from battle and returning to plaza.");
                try{
                    //Smoke Ball or Flying type required due to Arena Trap/Magnet Pull
                    NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                    battle_menu.move_to_slot(stream, context, 3);
                    pbf_press_button(context, BUTTON_A, 10, 50);
                    press_Bs_to_back_to_overworld(info, stream, context);
                    return_to_plaza(info, stream, context);
                }catch (...){
                    stream.log("Unable to flee.");
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Unable to flee!",
                        stream
                    );
                }
            }
        }
    }
    context.wait_for_all_requests();
    return_to_plaza(info, stream, context);
}

void quest_catch_navi(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
){
    switch (current_quest){
        case BBQuests::catch_any: case BBQuests::catch_normal: case BBQuests::catch_fire:
            stream.log("Catch: Any/Normal/Fire");

            //Savanna Plaza - Pride Rock
            central_to_savanna_plaza(info, stream, context);

            //pbf_move_left_joystick(context, 255, 255, 10, 20);
            pbf_move_left_joystick(context, 220, 255, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 600, 400, 400);

            pbf_press_button(context, BUTTON_PLUS, 20, 105);
            pbf_move_left_joystick(context, 255, 128, 20, 50);

            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 128, 0, 100, 50);
            pbf_move_left_joystick(context, 0, 0, 20, 50);
            pbf_press_button(context, BUTTON_L, 20, 50);

            break;

        case BBQuests::catch_psychic:
            stream.log("Catch: Psychic");

            //Polar Rest Area - targeting Duosion fixed spawn
            central_to_polar_rest(info, stream, context);

            pbf_press_button(context, BUTTON_L, 10, 50);
            pbf_move_left_joystick(context, 128, 0, 230, 20);
            pbf_move_left_joystick(context, 0, 128, 250, 20);

            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 128, 0, 150, 20);

            break;

        case BBQuests::catch_grass: case BBQuests::catch_dragon:
            stream.log("Catch: Grass/Dragon");

            //Coastal Plaza - Exeggutor-A
            central_to_coastal_plaza(info, stream, context);

            pbf_move_left_joystick(context, 0, 115, 400, 20);

            //Jump down
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);
            ssf_press_button(context, BUTTON_B, 0, 100);
            ssf_press_button(context, BUTTON_B, 0, 20, 10);
            ssf_press_button(context, BUTTON_B, 0, 20);

            pbf_wait(context, 100);
            context.wait_for_all_requests();

            pbf_move_left_joystick(context, 128, 0, 350, 20);
            pbf_press_button(context, BUTTON_B, 20, 20);
            pbf_wait(context, 200);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            break;
        case BBQuests::catch_ghost: case BBQuests::catch_ground:
            stream.log("Catch: Ghost/Ground");

            //Canyon Plaza - Golett
            central_to_canyon_plaza(info, stream, context);

            pbf_move_left_joystick(context, 210, 128, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 600, 300, 400);

            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            pbf_move_left_joystick(context, 0, 0, 10, 20);
            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 128, 0, 50, 20);

            break;
        case BBQuests::catch_fairy:
            stream.log("Catch: Fairy");

            //Snubbull - Central plaza
            //Tested while snowing, seems fine?
            open_map_from_overworld(info, stream, context);
            fly_to_overworld_from_map(info, stream, context);

            pbf_move_left_joystick(context, 0, 80, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 2000, 1500, 200);

            pbf_press_button(context, BUTTON_PLUS, 20, 105);

            pbf_move_left_joystick(context, 255, 0, 10, 20);
            pbf_press_button(context, BUTTON_L, 20, 50);

            break;
        case BBQuests::catch_fighting:
            stream.log("Catch: Fighting");

            //Hitmontop (TERA ICE) - Canyon Plaza or Classroom
            central_to_canyon_plaza(info, stream, context);
            pbf_move_left_joystick(context, 0, 128, 400, 20);
            pbf_press_button(context, BUTTON_L, 10, 50);
            pbf_move_left_joystick(context, 0, 100, 20, 50);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 200, 500, 800);

            break;

        case BBQuests::catch_bug:
            stream.log("Catch: Bug");

            central_to_canyon_plaza(info, stream, context);

            pbf_move_left_joystick(context, 205, 64, 20, 105);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 1000, 1650, 500);
            break;
        case BBQuests::catch_dark: case BBQuests::catch_flying:
            stream.log("Catch: Dark/Flying");

            //Vullaby/Mandibuzz
            central_to_savanna_plaza(info, stream, context);

            pbf_move_left_joystick(context, 255, 40, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            pbf_move_left_joystick(context, 128, 0, 500, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            pbf_move_left_joystick(context, 255, 0, 10, 20);
            pbf_press_button(context, BUTTON_L, 20, 50);

            pbf_move_left_joystick(context, 128, 0, 200, 20);
            pbf_press_button(context, BUTTON_L, 20, 50);

            pbf_move_left_joystick(context, 0, 0, 10, 20);
            pbf_press_button(context, BUTTON_L, 20, 50);

            pbf_move_left_joystick(context, 128, 0, 100, 20);
            pbf_wait(context, 400);
            context.wait_for_all_requests();

            break;
        case BBQuests::catch_rock: case BBQuests::catch_electric:
            stream.log("Catch: Rock/Electric");

            //Geodude-A
            open_map_from_overworld(info, stream, context);
            fly_to_overworld_from_map(info, stream, context);
            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 70, 0, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 100, 550, 300);

            pbf_press_button(context, BUTTON_PLUS, 20, 105);
            pbf_wait(context, 300);
            context.wait_for_all_requests();

            break;
        case BBQuests::catch_steel:
            stream.log("Catch: Steel");

            //Dugtrio-A - area a bit laggy but should work most of the time
            central_to_coastal_plaza(info, stream, context);
            pbf_move_left_joystick(context, 0, 128, 20, 50);

            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);
            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 200, 590, 200);

            pbf_press_button(context, BUTTON_PLUS, 20, 105);
            pbf_move_left_joystick(context, 0, 128, 20, 50);
            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 128, 0, 50, 50);

            break;
        case BBQuests::catch_poison:
            stream.log("Catch: Poison");

            //Muk-A - area a bit laggy but consistently so
            central_to_coastal_plaza(info, stream, context);
            pbf_move_left_joystick(context, 0, 128, 20, 50);

            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);
            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 1000, 1800, 300);

            pbf_press_button(context, BUTTON_PLUS, 20, 105);

            //Extra throws for this one
            ssf_press_button(context, BUTTON_ZR, 0, 200);
            ssf_press_button(context, BUTTON_ZL, 100, 50);
            ssf_press_button(context, BUTTON_ZL, 150, 50);
            pbf_wait(context, 200);
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_ZR, 20, 50); //Withdraw pokemon

            pbf_move_left_joystick(context, 255, 128, 20, 50);
            pbf_press_button(context, BUTTON_L, 20, 50);
            ssf_press_button(context, BUTTON_ZR, 0, 200);
            ssf_press_button(context, BUTTON_ZL, 100, 50);
            ssf_press_button(context, BUTTON_ZL, 150, 50);
            pbf_wait(context, 200);
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_ZR, 20, 50); //Withdraw pokemon

            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 255, 128, 20, 50);
            pbf_press_button(context, BUTTON_L, 20, 50);

            break;
        case BBQuests::catch_water: case BBQuests::catch_ice:
            stream.log("Catch: Water/Ice");

            //Lapras - Tera Bug
            central_to_polar_rest(info, stream, context);
            pbf_press_button(context, BUTTON_L, 10, 50);
            pbf_move_left_joystick(context, 128, 0, 230, 20);
            pbf_move_left_joystick(context, 0, 128, 300, 20);
            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 20, 0, 20, 50);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            jump_glide_fly(stream, context, BBQ_OPTIONS.INVERTED_FLIGHT, 700, 1700, 300);

            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Invalid catch quest.",
                stream
            );
            break;
    }

    //Lock on and throw ball
    ssf_press_button(context, BUTTON_ZR, 0, 200);
    ssf_press_button(context, BUTTON_ZL, 100, 50);
    ssf_press_button(context, BUTTON_ZL, 150, 50);

    pbf_wait(context, 300);
    context.wait_for_all_requests();

}

void quest_catch_throw_ball(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    Language language,
    const std::string& selected_ball
){
    BattleBallReader reader(stream, language);
    std::string ball_reader = "";
    WallClock start = current_time();

    stream.log("Opening ball menu...");
    while (ball_reader == ""){
        if (current_time() - start > std::chrono::minutes(2)){
            stream.log("Timed out trying to read ball after 2 minutes.", COLOR_RED);
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Timed out trying to read ball after 2 minutes.",
                stream
            );
        }

        //Mash B to exit anything else
        pbf_mash_button(context, BUTTON_B, 125);
        context.wait_for_all_requests();

        //Press X to open Ball menu
        pbf_press_button(context, BUTTON_X, 20, 100);
        context.wait_for_all_requests();

        VideoSnapshot screen = stream.video().snapshot();
        ball_reader = reader.read_ball(screen);
    }

    stream.log("Selecting ball.");
    int quantity = move_to_ball(reader, stream, context, selected_ball);
    if (quantity == 0){
        stream.log("Unable to find ball.");
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to find ball.",
            stream
        );
    }
    if (quantity < 0){
        stream.log("Unable to read ball quantity.", COLOR_RED);
    }

    //Throw ball
    stream.log("Throwing ball.");
    pbf_mash_button(context, BUTTON_A, 150);
    context.wait_for_all_requests();

    pbf_mash_button(context, BUTTON_B, 900);
    context.wait_for_all_requests();
}

void quest_catch_handle_battle(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
){
    stream.log("Catching Pokemon.");
    AdvanceDialogWatcher advance_dialog(COLOR_MAGENTA);
    PromptDialogWatcher add_to_party(COLOR_PURPLE);
    OverworldWatcher overworld(stream.logger(), COLOR_RED);

    uint8_t switch_party_slot = 1;
    bool quickball_thrown = false;
    bool tera_target = false;
    bool use_quickball = BBQ_OPTIONS.QUICKBALL;

    int ret2 = run_until<SwitchControllerContext>(
        stream, context,
        [&](SwitchControllerContext& context){
            while (true){
                //Check that battle menu appears - this is in case of swapping pokemon
                NormalBattleMenuWatcher menu_before_throw(COLOR_YELLOW);
                int bMenu = wait_until(
                    stream, context,
                    std::chrono::seconds(15),
                    { menu_before_throw }
                );
                if (bMenu < 0){
                    stream.log("Unable to find menu_before_throw.");
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Unable to find menu_before_throw.",
                        stream
                    );
                }
                
                //Check for catch menu - if none it is a tera pokemon
                stream.log("Checking for ball menu.");
                BattleBallReader exists(stream, BBQ_OPTIONS.LANGUAGE);
                std::string ball_exists = "";

                pbf_press_button(context, BUTTON_X, 20, 100);
                context.wait_for_all_requests();

                VideoSnapshot screen_ball = stream.video().snapshot();
                ball_exists = exists.read_ball(screen_ball);

                if (ball_exists == ""){
                    stream.log("Could not find ball reader. Tera battle. Using first attack.");
                    pbf_mash_button(context, BUTTON_B, 125);
                    context.wait_for_all_requests();

                    pbf_mash_button(context, BUTTON_A, 300);
                    context.wait_for_all_requests();

                    tera_target = true;
                }else{
                    //Quick ball occurs before anything else in battle.
                    //Do not throw if target is a tera pokemon.
                    if (use_quickball && !quickball_thrown && tera_target == false){
                        stream.log("Quick Ball option checked. Throwing Quick Ball.");
                        quest_catch_throw_ball(info, stream, context, BBQ_OPTIONS.LANGUAGE, "quick-ball");
                        quickball_thrown = true;
                    }else{
                        stream.log("Throwing selected ball.");
                        quest_catch_throw_ball(info, stream, context, BBQ_OPTIONS.LANGUAGE, BBQ_OPTIONS.BALL_SELECT.slug());
                    }

                    //Check for battle menu, if found use fourth attack this turn
                    NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                    int ret = wait_until(
                        stream, context,
                        std::chrono::seconds(4),
                        { battle_menu }
                    );
                    if (ret == 0){
                        stream.log("Battle menu detected early. Using fourth attack.");
                        MoveSelectWatcher move_watcher(COLOR_BLUE);
                        MoveSelectDetector move_select(COLOR_BLUE);

                        int ret_move_select = run_until<SwitchControllerContext>(
                            stream, context,
                            [&](SwitchControllerContext& context){
                                pbf_press_button(context, BUTTON_A, 10, 50);
                                pbf_wait(context, 100);
                                context.wait_for_all_requests();
                            },
                            { move_watcher }
                        );
                        if (ret_move_select != 0){
                            stream.log("Could not find move select.");
                        }else{
                            stream.log("Move select found!");
                        }

                        context.wait_for_all_requests();
                        move_select.move_to_slot(stream, context, 3);
                        pbf_mash_button(context, BUTTON_A, 150);
                        pbf_wait(context, 100);
                        context.wait_for_all_requests();

                        //Check for battle menu
                        //If found after a second, assume out of PP and stop as this is a setup issue
                        //None of the target pokemon for this program have disable, taunt, etc.
                        NormalBattleMenuWatcher battle_menu2(COLOR_YELLOW);
                        int ret3 = wait_until(
                            stream, context,
                            std::chrono::seconds(4),
                            { battle_menu2 }
                        );
                        if (ret3 == 0){
                            stream.log("Battle menu detected early. Out of PP/No move in slot, please check your setup.");
                            OperationFailedException::fire(
                                ErrorReport::SEND_ERROR_REPORT,
                                "Battle menu detected early. Out of PP, please check your setup.",
                                stream
                            );
                        }
                    }else{
                        //Wild pokemon's turn/wait for catch animation
                        pbf_mash_button(context, BUTTON_B, 900);
                        context.wait_for_all_requests();
                    }
                }

                NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                GradientArrowWatcher fainted(COLOR_BLUE, GradientArrowType::RIGHT, {0.610, 0.523, 0.044, 0.079});
                SwapMenuWatcher swap(COLOR_YELLOW);
                int ret2_run = wait_until(
                    stream, context,
                    std::chrono::seconds(60),
                    { battle_menu, fainted }
                );
                switch (ret2_run){
                case 0:
                    stream.log("Battle menu detected, continuing.");
                    break;
                case 1:
                    stream.log("Detected fainted Pokemon. Switching to next living Pokemon...");
                    pbf_press_button(context, BUTTON_A, 20, 50);
                    pbf_wait(context, 100);
                    context.wait_for_all_requests();
                    if (swap.move_to_slot(stream, context, switch_party_slot)){
                        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
                        context.wait_for_all_requests();
                        switch_party_slot++;
                    }
                    break;
                default:
                    stream.log("Invalid state ret2_run. Out of moves?");
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Invalid state ret2_run. Out of moves?",
                        stream
                    );
                }

            }
        },
        { advance_dialog, add_to_party, overworld }
        );

    switch (ret2){
    case 0:
        stream.log("Advance Dialog detected.");
        press_Bs_to_back_to_overworld(info, stream, context);
        break;
    case 1:
        stream.log("Prompt dialog detected.");
        press_Bs_to_back_to_overworld(info, stream, context);
        break;
    case 2:
        stream.log("Overworld detected.");
        break;
    default:
        stream.log("Invalid state in run_battle().");
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Invalid state in run_battle().",
            stream
        );
    }
}

void quest_catch(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    const BBQOption& BBQ_OPTIONS,
    BBQuests current_quest
){
    EncounterWatcher encounter_watcher(stream, COLOR_RED);

    //Navigate to target and start battle
    int ret = run_until<SwitchControllerContext>(
        stream, context,
        [&](SwitchControllerContext& context){
            
            quest_catch_navi(info, stream, context, BBQ_OPTIONS, current_quest);
            context.wait_for_all_requests();

            NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
            int ret2 = wait_until(
                stream, context,
                std::chrono::seconds(25),
                { battle_menu }
            );
            if (ret2 != 0){
                stream.log("Did not enter battle. Did target spawn?");
            }
        },
        {
            static_cast<VisualInferenceCallback&>(encounter_watcher),
            static_cast<AudioInferenceCallback&>(encounter_watcher),
        }
        );
    encounter_watcher.throw_if_no_sound();

    if (ret >= 0){
        stream.log("Battle menu detected.");

        bool is_shiny = (bool)encounter_watcher.shiny_screenshot();
        if (is_shiny){
            stream.log("Shiny detected!");
            pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
            throw ProgramFinishedException();
        }else{
            quest_catch_handle_battle(info, stream, context, BBQ_OPTIONS, current_quest);
        }
    }

    return_to_plaza(info, stream, context);

    //Day skip and attempt to respawn fixed encounters
    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    home_to_date_time(context, true, true);
    PokemonSwSh::roll_date_forward_1(context, true);
    resume_game_from_home(stream, context);

    //Heal up and then reset position again.
    OverworldWatcher done_healing(stream.logger(), COLOR_BLUE);
    pbf_move_left_joystick(context, 128, 0, 100, 20);

    pbf_mash_button(context, BUTTON_A, 300);
    context.wait_for_all_requests();

    int exit = run_until<SwitchControllerContext>(
        stream, context,
        [&](SwitchControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 2000);
        },
        {{ done_healing }}
    );
    if (exit == 0){
        stream.log("Overworld detected.");
    }
    open_map_from_overworld(info, stream, context);
    pbf_press_button(context, BUTTON_ZL, 40, 100);
    fly_to_overworld_from_map(info, stream, context);
    context.wait_for_all_requests();
}

void wild_battle_tera(
    const ProgramInfo& info,
    VideoStream& stream, SwitchControllerContext& context,
    bool& tera_self
){
    AdvanceDialogWatcher lost(COLOR_YELLOW);
    OverworldWatcher overworld(stream.logger(), COLOR_RED);
    WallClock start = current_time();
    uint8_t switch_party_slot = 1;
    bool first_turn = true;

    int ret2 = run_until<SwitchControllerContext>(
        stream, context,
        [&](SwitchControllerContext& context){
            while(true){
                if (current_time() - start > std::chrono::minutes(5)){
                    stream.log("Timed out during battle after 5 minutes.", COLOR_RED);
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Timed out during battle after 5 minutes.",
                        stream
                    );
                }

                if (first_turn && tera_self){
                    stream.log("Turn 1: Tera.");
                    //Open move menu
                    pbf_press_button(context, BUTTON_A, 10, 50);
                    pbf_wait(context, 100);
                    context.wait_for_all_requests();

                    pbf_press_button(context, BUTTON_R, 20, 50);
                    pbf_press_button(context, BUTTON_A, 10, 50);

                    first_turn = false;
                }

                NormalBattleMenuWatcher battle_menu(COLOR_MAGENTA);
                GradientArrowWatcher fainted(COLOR_BLUE, GradientArrowType::RIGHT, {0.610, 0.523, 0.044, 0.079});
                SwapMenuWatcher swap(COLOR_YELLOW);

                context.wait_for_all_requests();

                int ret3 = wait_until(
                    stream, context,
                    std::chrono::seconds(90),
                    { battle_menu, fainted }
                );
                switch (ret3){
                case 0:
                    stream.log("Detected battle menu. Pressing A to attack...");
                    pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
                    context.wait_for_all_requests();
                    break;
                case 1:
                    stream.log("Detected fainted Pokemon. Switching to next living Pokemon...");
                    pbf_press_button(context, BUTTON_A, 20, 50);
                    pbf_wait(context, 100);
                    context.wait_for_all_requests();
                    if (swap.move_to_slot(stream, context, switch_party_slot)){
                        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
                        context.wait_for_all_requests();
                        switch_party_slot++;
                    }
                    break;
                default:
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Timed out during battle. Stuck, crashed, or took more than 90 seconds for a turn.",
                        stream
                    );
                }
            }
        },
        { lost, overworld }
    );
    if (ret2 == 0){
        stream.log("Lost battle. Mashing B.");
    }
}

}
}
}
