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


void VirtualControllerState::print() const{
    cout << "dpad = (" << dpad_x << "," << dpad_y
         << "), left = (" << left_joystick_x << "," << left_joystick_y
         << "), right = (" << right_joystick_x << "," << right_joystick_y
         << "), buttons =";
    if (buttons & BUTTON_Y) cout << " Y";
    if (buttons & BUTTON_B) cout << " B";
    if (buttons & BUTTON_A) cout << " A";
    if (buttons & BUTTON_X) cout << " X";
    if (buttons & BUTTON_L) cout << " L";
    if (buttons & BUTTON_R) cout << " R";
    if (buttons & BUTTON_ZL) cout << " ZL";
    if (buttons & BUTTON_ZR) cout << " ZR";
    if (buttons & BUTTON_PLUS) cout << " +";
    if (buttons & BUTTON_MINUS) cout << " -";
    if (buttons & BUTTON_LCLICK) cout << " LC";
    if (buttons & BUTTON_RCLICK) cout << " RC";
    if (buttons & BUTTON_HOME) cout << " Home";
    if (buttons & BUTTON_CAPTURE) cout << " Screen";
    cout << ")" << endl;
}

bool VirtualControllerState::operator==(const VirtualControllerState& x) const{
    if (buttons != x.buttons){
        return false;
    }
    if (dpad_x != x.dpad_x){
        return false;
    }
    if (dpad_y != x.dpad_y){
        return false;
    }
    if (left_joystick_x != x.left_joystick_x){
        return false;
    }
    if (left_joystick_y != x.left_joystick_y){
        return false;
    }
    if (right_joystick_x != x.right_joystick_x){
        return false;
    }
    if (right_joystick_y != x.right_joystick_y){
        return false;
    }
    return true;
}
bool VirtualControllerState::operator!=(const VirtualControllerState& x) const{
    return !(*this == x);
}


bool VirtualControllerState::to_state(ControllerState& state) const{
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
    if (left_joystick_x != 0 || left_joystick_y != 0){
        neutral = false;
        int mag = std::abs(left_joystick_x) > std::abs(left_joystick_y)
            ? std::abs(left_joystick_x)
            : std::abs(left_joystick_y);
        state.left_x = (uint8_t)std::min(128 * left_joystick_x / mag + 128, 255);
        state.left_y = (uint8_t)std::min(128 * left_joystick_y / mag + 128, 255);
    }

    state.right_x = 128;
    state.right_y = 128;
    if (right_joystick_x != 0 || right_joystick_y != 0){
        neutral = false;
        int mag = std::abs(right_joystick_x) > std::abs(right_joystick_y)
            ? std::abs(right_joystick_x)
            : std::abs(right_joystick_y);
        state.right_x = (uint8_t)std::min(128 * right_joystick_x / mag + 128, 255);
        state.right_y = (uint8_t)std::min(128 * right_joystick_y / mag + 128, 255);
    }

    return neutral;
}





}
}
