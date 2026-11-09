/*  SDL Gamepad Input
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_Gamepad_GamepadInput_SDL_H
#define PokemonAutomation_ControllerInput_Gamepad_GamepadInput_SDL_H

#ifdef PA_SDL

#include <tuple>
#include <SDL3/SDL.h>
#include "GamepadInput_State.h"

namespace PokemonAutomation{

std::tuple<double, double> gamepad_stick(int x, int y);
bool gamepad_trigger(int value);
GamepadInputState read_gamepad(SDL_Gamepad* pad);

}
#endif
#endif
