/*  Regi Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh_ShinyHunt-Regi.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void move_to_corner(
    Logger& logger, ProControllerContext& context,
    bool correction, Milliseconds TRANSITION_DELAY
){
    if (correction){
        logger.log("Performing auto-correction.");
        //  Move down to building exit and exit.
        pbf_move_left_joystick(context, 128, 255, 4000ms, TRANSITION_DELAY);
        pbf_move_left_joystick(context, 128, 255, 2400ms, TRANSITION_DELAY);

        //  Navigate back into the corner.
        pbf_move_left_joystick(context, 255, 64, 1600ms, 0ms);
        pbf_move_left_joystick(context, 120, 0, 2000ms, 0ms);
        pbf_move_left_joystick(context, 255, 100, 1200ms, 80ms);
    }else{
        //  Move to corner.
        pbf_move_left_joystick(context, 255, 100, 2400ms, 80ms);
    }
}


void regirock(ProControllerContext& context){
    pbf_move_left_joystick(context, 0, 232, 1352ms, 0ms);
    pbf_move_left_joystick(context, 128, 255, 1080ms, 0ms);
    pbf_move_left_joystick(context, {-1, 0}, 872ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 1120ms, 0ms);
    pbf_move_left_joystick(context, 226, 255, 720ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 160ms, 0ms);
    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_move_left_joystick(context, 128, 0, 1600ms, 0ms);
}
void regice(ProControllerContext& context){
    pbf_move_left_joystick(context, 80, 255, 1456ms, 0ms);
    pbf_move_left_joystick(context, {-1, 0}, 912ms, 0ms);
    pbf_move_left_joystick(context, 128, 255, 448ms, 0ms);
    pbf_move_left_joystick(context, 32, 0, 608ms, 0ms);
    pbf_move_left_joystick(context, {-1, 0}, 432ms, 0ms);
    pbf_move_left_joystick(context, 255, 68, 1224ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 160ms, 0ms);
    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_move_left_joystick(context, 128, 0, 1360ms, 0ms);
}
void registeel(ProControllerContext& context){
    pbf_move_left_joystick(context, 0, 232, 1352ms, 0ms);
    pbf_move_left_joystick(context, 192, 255, 512ms, 0ms);
    pbf_move_left_joystick(context, 64, 255, 512ms, 0ms);
    pbf_move_left_joystick(context, {-1, 0}, 880ms, 0ms);
    pbf_move_left_joystick(context, 64, 0, 544ms, 0ms);
    pbf_move_left_joystick(context, 192, 0, 528ms, 0ms);
    pbf_move_left_joystick(context, 230, 255, 720ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 400ms, 0ms);
    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_move_left_joystick(context, 128, 0, 2000ms, 0ms);
}
void regieleki(ProControllerContext& context){
    pbf_move_left_joystick(context, 16, 255, 1296ms, 0ms);
    pbf_move_left_joystick(context, 64, 255, 416ms, 0ms);
    pbf_move_left_joystick(context, 200, 255, 416ms, 0ms);
    pbf_move_left_joystick(context, 0, 50, 976ms, 0ms);
    pbf_move_left_joystick(context, 0, 206, 744ms, 0ms);
    pbf_move_left_joystick(context, 208, 0, 624ms, 0ms);
    pbf_move_left_joystick(context, 60, 0, 640ms, 0ms);
    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_move_left_joystick(context, 216, 0, 1360ms, 0ms);
}
void regidrago(ProControllerContext& context){
    pbf_move_left_joystick(context, 16, 255, 1280ms, 0ms);
    pbf_move_left_joystick(context, 72, 255, 400ms, 0ms);
    pbf_move_left_joystick(context, 48, 255, 848ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 384ms, 0ms);
    pbf_move_left_joystick(context, 0, 56, 480ms, 0ms);
    pbf_move_left_joystick(context, 64, 0, 480ms, 0ms);
    pbf_move_left_joystick(context, 255, 152, 640ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 160ms, 0ms);
    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_move_left_joystick(context, 128, 0, 1440ms, 0ms);
}

void run_regi_light_puzzle(
    Logger& logger, ProControllerContext& context,
    RegiGolem regi, uint64_t encounter
){
    switch (regi){
    case RegiGolem::Regirock:
        regirock(context);
        logger.log("Starting Regirock Encounter: " + tostr_u_commas(encounter + 1));
        break;
    case RegiGolem::Regice:
        regice(context);
        logger.log("Starting Regice Encounter: " + tostr_u_commas(encounter + 1));
        break;
    case RegiGolem::Registeel:
        registeel(context);
        logger.log("Starting Registeel Encounter: " + tostr_u_commas(encounter + 1));
        break;
    case RegiGolem::Regieleki:
        regieleki(context);
        logger.log("Starting Regieleki Encounter: " + tostr_u_commas(encounter + 1));
        break;
    case RegiGolem::Regidrago:
        regidrago(context);
        logger.log("Starting Regidrago Encounter: " + tostr_u_commas(encounter + 1));
        break;
    }
}


}
}
}

