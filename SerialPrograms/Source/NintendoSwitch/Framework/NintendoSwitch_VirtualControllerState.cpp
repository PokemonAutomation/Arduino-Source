/*  Virtual Controller State
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch_VirtualControllerState.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



bool ControllerState::operator==(const ControllerState& x) const{
    if (buttons != x.buttons){
        return false;
    }
    if (dpad != x.dpad){
        return false;
    }
    if (left_x != x.left_x){
        return false;
    }
    if (left_y != x.left_y){
        return false;
    }
    if (right_x != x.right_x){
        return false;
    }
    if (right_y != x.right_y){
        return false;
    }
    return true;
}
bool ControllerState::operator!=(const ControllerState& x) const{
    return !(*this == x);
}

void ControllerDeltas::operator+=(const ControllerDeltas& x){
    buttons |= x.buttons;
    dpad_x += x.dpad_x;
    dpad_y += x.dpad_y;
    left_x += x.left_x;
    left_y += x.left_y;
    right_x += x.right_x;
    right_y += x.right_y;
}
bool ControllerDeltas::to_state(ControllerState& state) const{
    bool neutral;

    state.buttons = buttons;
    neutral = buttons == 0;

    state.dpad = DPAD_NONE;
    if (dpad_x != 0 || dpad_y != 0){
        neutral = false;
        do{
            if (dpad_x == 0){
                state.dpad = dpad_y > 0 ? DPAD_DOWN : DPAD_UP;
                break;
            }
            if (dpad_y == 0){
                state.dpad = dpad_x > 0 ? DPAD_RIGHT : DPAD_LEFT;
                break;
            }
            if (dpad_x < 0 && dpad_y < 0){
                state.dpad = DPAD_UP_LEFT;
                break;
            }
            if (dpad_x < 0 && dpad_y > 0){
                state.dpad = DPAD_DOWN_LEFT;
                break;
            }
            if (dpad_x > 0 && dpad_y > 0){
                state.dpad = DPAD_DOWN_RIGHT;
                break;
            }
            if (dpad_x > 0 && dpad_y < 0){
                state.dpad = DPAD_UP_RIGHT;
                break;
            }
        }while (false);
    }

    state.left_x = 128;
    state.left_y = 128;
    if (left_x != 0 || left_y != 0){
        neutral = false;
        int mag = std::abs(left_x) > std::abs(left_y)
            ? std::abs(left_x)
            : std::abs(left_y);
        state.left_x = (uint8_t)std::min(128 * left_x / mag + 128, 255);
        state.left_y = (uint8_t)std::min(128 * left_y / mag + 128, 255);
    }

    state.right_x = 128;
    state.right_y = 128;
    if (right_x != 0 || right_y != 0){
        neutral = false;
        int mag = std::abs(right_x) > std::abs(right_y)
            ? std::abs(right_x)
            : std::abs(right_y);
        state.right_x = (uint8_t)std::min(128 * right_x / mag + 128, 255);
        state.right_y = (uint8_t)std::min(128 * right_y / mag + 128, 255);
    }

    return neutral;
}











}
}
