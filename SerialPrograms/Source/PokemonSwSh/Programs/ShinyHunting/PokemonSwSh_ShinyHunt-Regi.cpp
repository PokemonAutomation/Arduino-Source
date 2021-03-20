/*  Regi Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "PokemonSwSh_ShinyHunt-Regi.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void move_to_corner(
    SingleSwitchProgramEnvironment& env,
    bool correction, uint16_t TRANSITION_DELAY
){
    if (correction){
        env.logger.log("Performing auto-correction.");
        //  Move down to building exit and exit.
        pbf_move_left_joystick(env.console, 128, 255, 500, TRANSITION_DELAY);
        pbf_move_left_joystick(env.console, 128, 255, 300, TRANSITION_DELAY);

        //  Navigate back into the corner.
        pbf_move_left_joystick(env.console, 255, 64, 200, 0);
        pbf_move_left_joystick(env.console, 120, 0, 250, 0);
        pbf_move_left_joystick(env.console, 255, 100, 150, 10);
    }else{
        //  Move to corner.
        pbf_move_left_joystick(env.console, 255, 100, 300, 10);
    }
}


void regirock(BotBase& botbase){
    pbf_move_left_joystick(botbase, 0, 232, 169, 0);
    pbf_move_left_joystick(botbase, 128, 255, 135, 0);
    pbf_move_left_joystick(botbase, 0, 128, 109, 0);
    pbf_move_left_joystick(botbase, 128, 0, 140, 0);
    pbf_move_left_joystick(botbase, 226, 255, 90, 0);
    pbf_move_left_joystick(botbase, 128, 0, 20, 0);
    pbf_mash_button(botbase, BUTTON_A, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(botbase, 128, 0, 200, 0);
}
void regice(BotBase& botbase){
    pbf_move_left_joystick(botbase, 80, 255, 182, 0);
    pbf_move_left_joystick(botbase, 0, 128, 114, 0);
    pbf_move_left_joystick(botbase, 128, 255, 56, 0);
    pbf_move_left_joystick(botbase, 32, 0, 76, 0);
    pbf_move_left_joystick(botbase, 0, 128, 54, 0);
    pbf_move_left_joystick(botbase, 255, 68, 154, 0);
    pbf_move_left_joystick(botbase, 128, 0, 20, 0);
    pbf_mash_button(botbase, BUTTON_A, 5 * TICKS_PER_SECOND - 20);
    pbf_move_left_joystick(botbase, 128, 0, 170, 0);
}
void registeel(BotBase& botbase){
    pbf_move_left_joystick(botbase, 0, 232, 169, 0);
    pbf_move_left_joystick(botbase, 192, 255, 64, 0);
    pbf_move_left_joystick(botbase, 64, 255, 64, 0);
    pbf_move_left_joystick(botbase, 0, 128, 110, 0);
    pbf_move_left_joystick(botbase, 64, 0, 68, 0);
    pbf_move_left_joystick(botbase, 192, 0, 66, 0);
    pbf_move_left_joystick(botbase, 230, 255, 90, 0);
    pbf_move_left_joystick(botbase, 128, 0, 50, 0);
    pbf_mash_button(botbase, BUTTON_A, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(botbase, 128, 0, 250, 0);
}
void regieleki(BotBase& botbase){
    pbf_move_left_joystick(botbase, 16, 255, 162, 0);
    pbf_move_left_joystick(botbase, 64, 255, 52, 0);
    pbf_move_left_joystick(botbase, 200, 255, 52, 0);
    pbf_move_left_joystick(botbase, 0, 50, 122, 0);
    pbf_move_left_joystick(botbase, 0, 206, 93, 0);
    pbf_move_left_joystick(botbase, 216, 0, 78, 0);
    pbf_move_left_joystick(botbase, 56, 0, 80, 0);
    pbf_mash_button(botbase, BUTTON_A, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(botbase, 216, 0, 170, 0);
}
void regidrago(BotBase& botbase){
    pbf_move_left_joystick(botbase, 16, 255, 160, 0);
    pbf_move_left_joystick(botbase, 72, 255, 50, 0);
    pbf_move_left_joystick(botbase, 48, 255, 104, 0);
    pbf_move_left_joystick(botbase, 128, 0, 48, 0);
    pbf_move_left_joystick(botbase, 0, 56, 60, 0);
    pbf_move_left_joystick(botbase, 64, 0, 60, 0);
    pbf_move_left_joystick(botbase, 255, 148, 75, 0);
    pbf_mash_button(botbase, BUTTON_A, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(botbase, 128, 0, 180, 0);
}

void run_regi_light_puzzle(
    SingleSwitchProgramEnvironment& env,
    RegiGolem regi, uint64_t encounter
){
    switch (regi){
    case RegiGolem::Regirock:
        regirock(env.console);
        env.logger.log("Starting Regirock Encounter: " + tostr_u_commas(encounter + 1));
        break;
    case RegiGolem::Regice:
        regice(env.console);
        env.logger.log("Starting Regice Encounter: " + tostr_u_commas(encounter + 1));
        break;
    case RegiGolem::Registeel:
        registeel(env.console);
        env.logger.log("Starting Registeel Encounter: " + tostr_u_commas(encounter + 1));
        break;
    case RegiGolem::Regieleki:
        regieleki(env.console);
        env.logger.log("Starting Regieleki Encounter: " + tostr_u_commas(encounter + 1));
        break;
    case RegiGolem::Regidrago:
        regidrago(env.console);
        env.logger.log("Starting Regidrago Encounter: " + tostr_u_commas(encounter + 1));
        break;
    }
}


}
}
}

