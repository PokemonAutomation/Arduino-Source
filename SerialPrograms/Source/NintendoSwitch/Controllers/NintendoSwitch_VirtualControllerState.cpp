/*  Virtual Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/Json/JsonObject.h"
#include "Procon/NintendoSwitch_ProControllerState.h"
#include "Joycon/NintendoSwitch_JoyconState.h"
#include "NintendoSwitch_VirtualControllerState.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;







void ProControllerDeltas::operator+=(const ProControllerDeltas& x){
    buttons |= x.buttons;
    dpad_x += x.dpad_x;
    dpad_y += x.dpad_y;
    left_x += x.left_x;
    left_y += x.left_y;
    right_x += x.right_x;
    right_y += x.right_y;
}
bool ProControllerDeltas::to_state(ProControllerState& state) const{
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

    state.left_joystick.x = 0;
    state.left_joystick.y = 0;
    if (left_x != 0 || left_y != 0){
        neutral = false;
        int mag = std::abs(left_x) > std::abs(left_y)
            ? std::abs(left_x)
            : std::abs(left_y);
        state.left_joystick.x = (double)left_x / mag;
        state.left_joystick.y = -(double)left_y / mag;
    }

    state.right_joystick.x = 0;
    state.right_joystick.y = 0;
    if (right_x != 0 || right_y != 0){
        neutral = false;
        int mag = std::abs(right_x) > std::abs(right_y)
            ? std::abs(right_x)
            : std::abs(right_y);
        state.right_joystick.x = (double)right_x / mag;
        state.right_joystick.y = -(double)right_y / mag;
    }

    return neutral;
}







void JoyconDeltas::operator+=(const JoyconDeltas& x){
    buttons |= x.buttons;
    joystick_x += x.joystick_x;
    joystick_y += x.joystick_y;
}
bool JoyconDeltas::to_state(JoyconState& state) const{
    bool neutral;

    state.buttons = buttons;
    neutral = buttons == 0;

    state.joystick.x = 0;
    state.joystick.y = 0;
    if (joystick_x != 0 || joystick_y != 0){
        neutral = false;
        int mag = std::abs(joystick_x) > std::abs(joystick_y)
            ? std::abs(joystick_x)
            : std::abs(joystick_y);
        state.joystick.x = (double)joystick_x / mag;
        state.joystick.y = -(double)joystick_y / mag;
    }

    return neutral;
}











}
}
