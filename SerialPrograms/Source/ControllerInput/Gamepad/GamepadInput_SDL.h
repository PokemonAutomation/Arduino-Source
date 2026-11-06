/*  SDL Gamepad Input
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_Gamepad_GamepadInput_SDL_H
#define PokemonAutomation_ControllerInput_Gamepad_GamepadInput_SDL_H

#include <algorithm>
#include <cmath>
#include <tuple>
#include <utility>
#include <SDL3/SDL.h>
#include "GamepadInput_State.h"

namespace PokemonAutomation{

// Rescale outside a radial deadzone so centered sticks are exactly neutral,
// while retaining the entire analog range. SDL's positive Y points down.
inline std::tuple<double, double> gamepad_stick(int x, int y){
    constexpr double joystick_max_negative = -SDL_JOYSTICK_AXIS_MIN;
    constexpr double joystick_max_positive = SDL_JOYSTICK_AXIS_MAX;
    constexpr double deadzone = 0.15;

    // First normalize to [-1, 1]
    double normalized_x = x / (x < 0 ? joystick_max_negative : joystick_max_positive);
    double normalized_y = y / (y < 0 ? joystick_max_negative : joystick_max_positive);

    // Then apply a radial deadzone
    double magnitude = std::hypot(normalized_x, normalized_y);
    if (magnitude <= deadzone){
        return {0, 0};
    }

    // Rescale to the full range outside the deadzone
    double output_magnitude = (std::min(magnitude, 1.0) - deadzone) / (1.0 - deadzone);
    double scale = output_magnitude / magnitude;
    return {normalized_x * scale, -normalized_y * scale};
}

inline bool gamepad_trigger(int value){
    constexpr double joystick_max = SDL_JOYSTICK_AXIS_MAX;
    constexpr double deadzone = 0.20;

    return value > (joystick_max * deadzone);
}

inline GamepadInputState read_gamepad(SDL_Gamepad* pad){
    using namespace NintendoSwitch;

    GamepadInputState state;
    ProControllerState& output = state.controller_state;
    // Match physical Nintendo positions, regardless of the input device's labels.
    static constexpr std::pair<SDL_GamepadButton, Button> mapping[] = {
        {SDL_GAMEPAD_BUTTON_SOUTH, BUTTON_B},
        {SDL_GAMEPAD_BUTTON_EAST, BUTTON_A},
        {SDL_GAMEPAD_BUTTON_WEST, BUTTON_Y},
        {SDL_GAMEPAD_BUTTON_NORTH, BUTTON_X},
        {SDL_GAMEPAD_BUTTON_BACK, BUTTON_MINUS},
        {SDL_GAMEPAD_BUTTON_GUIDE, BUTTON_HOME},
        {SDL_GAMEPAD_BUTTON_START, BUTTON_PLUS},
        {SDL_GAMEPAD_BUTTON_LEFT_STICK, BUTTON_LCLICK},
        {SDL_GAMEPAD_BUTTON_RIGHT_STICK, BUTTON_RCLICK},
        {SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, BUTTON_L},
        {SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, BUTTON_R},
        {SDL_GAMEPAD_BUTTON_MISC1, BUTTON_CAPTURE},
    };
    for (const auto& [sdl_button, button] : mapping){
        if (SDL_GetGamepadButton(pad, sdl_button)){
            output.buttons |= button;
        }
    }
    if (gamepad_trigger(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER))){
        output.buttons |= BUTTON_ZL;
    }
    if (gamepad_trigger(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER))){
        output.buttons |= BUTTON_ZR;
    }

    int dpad_x = (int)SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)
        - (int)SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    int dpad_y = (int)SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN)
        - (int)SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    static constexpr DpadPosition dpad_positions[3][3] = {
        {DPAD_UP_LEFT, DPAD_UP, DPAD_UP_RIGHT},
        {DPAD_LEFT, DPAD_NONE, DPAD_RIGHT},
        {DPAD_DOWN_LEFT, DPAD_DOWN, DPAD_DOWN_RIGHT},
    };
    output.dpad = dpad_positions[dpad_y + 1][dpad_x + 1];

    std::tie(output.left_joystick.x, output.left_joystick.y) = gamepad_stick(
        SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX),
        SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY)
    );
    std::tie(output.right_joystick.x, output.right_joystick.y) = gamepad_stick(
        SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTX),
        SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTY)
    );
    return state;
}

}
#endif
