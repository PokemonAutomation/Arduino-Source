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
        pbf_move_left_joystick(context, 255, 64, 200, 0);
        pbf_move_left_joystick(context, 120, 0, 250, 0);
        pbf_move_left_joystick(context, 255, 100, 150, 10);
    }else{
        //  Move to corner.
        pbf_move_left_joystick(context, 255, 100, 300, 10);
    }
}


void regirock(ProControllerContext& context){
    pbf_move_left_joystick(context, 0, 232, 169, 0);
    pbf_move_left_joystick(context, 128, 255, 135, 0);
    pbf_move_left_joystick(context, 0, 128, 109, 0);
    pbf_move_left_joystick(context, 128, 0, 140, 0);
    pbf_move_left_joystick(context, 226, 255, 90, 0);
    pbf_move_left_joystick(context, 128, 0, 20, 0);
    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_move_left_joystick(context, 128, 0, 200, 0);
}
void regice(ProControllerContext& context){
    pbf_move_left_joystick(context, 80, 255, 182, 0);
    pbf_move_left_joystick(context, 0, 128, 114, 0);
    pbf_move_left_joystick(context, 128, 255, 56, 0);
    pbf_move_left_joystick(context, 32, 0, 76, 0);
    pbf_move_left_joystick(context, 0, 128, 54, 0);
    pbf_move_left_joystick(context, 255, 68, 153, 0);
    pbf_move_left_joystick(context, 128, 0, 20, 0);
    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_move_left_joystick(context, 128, 0, 170, 0);
}
void registeel(ProControllerContext& context){
    pbf_move_left_joystick(context, 0, 232, 169, 0);
    pbf_move_left_joystick(context, 192, 255, 64, 0);
    pbf_move_left_joystick(context, 64, 255, 64, 0);
    pbf_move_left_joystick(context, 0, 128, 110, 0);
    pbf_move_left_joystick(context, 64, 0, 68, 0);
    pbf_move_left_joystick(context, 192, 0, 66, 0);
    pbf_move_left_joystick(context, 230, 255, 90, 0);
    pbf_move_left_joystick(context, 128, 0, 50, 0);
    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_move_left_joystick(context, 128, 0, 250, 0);
}
void regieleki(ProControllerContext& context){
    pbf_move_left_joystick(context, 16, 255, 162, 0);
    pbf_move_left_joystick(context, 64, 255, 52, 0);
    pbf_move_left_joystick(context, 200, 255, 52, 0);
    pbf_move_left_joystick(context, 0, 50, 122, 0);
    pbf_move_left_joystick(context, 0, 206, 93, 0);
    pbf_move_left_joystick(context, 208, 0, 78, 0);
    pbf_move_left_joystick(context, 60, 0, 80, 0);
    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_move_left_joystick(context, 216, 0, 170, 0);
}
void regidrago(ProControllerContext& context){
    pbf_move_left_joystick(context, 16, 255, 160, 0);
    pbf_move_left_joystick(context, 72, 255, 50, 0);
    pbf_move_left_joystick(context, 48, 255, 106, 0);
    pbf_move_left_joystick(context, 128, 0, 48, 0);
    pbf_move_left_joystick(context, 0, 56, 60, 0);
    pbf_move_left_joystick(context, 64, 0, 60, 0);
    pbf_move_left_joystick(context, 255, 152, 80, 0);
    pbf_move_left_joystick(context, 128, 0, 20, 0);
    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_move_left_joystick(context, 128, 0, 180, 0);
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

