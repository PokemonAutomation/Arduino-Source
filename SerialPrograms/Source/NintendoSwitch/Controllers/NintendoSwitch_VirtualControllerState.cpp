/*  Virtual Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch_VirtualControllerState.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;





void ProControllerState::clear(){
    buttons = BUTTON_NONE;
    dpad = DPAD_NONE;
    left_x = 128;
    left_y = 128;
    right_x = 128;
    right_y = 128;
}
bool ProControllerState::operator==(const ControllerState& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }

    const ProControllerState& r = static_cast<const ProControllerState&>(x);

    if (buttons != r.buttons){
        return false;
    }
    if (dpad != r.dpad){
        return false;
    }
    if (left_x != r.left_x){
        return false;
    }
    if (left_y != r.left_y){
        return false;
    }
    if (right_x != r.right_x){
        return false;
    }
    if (right_y != r.right_y){
        return false;
    }
    return true;
}
bool ProControllerState::is_neutral() const{
    return buttons == 0
        && dpad == DPAD_NONE
        && left_x == 128
        && left_y == 128
        && right_x == 128
        && right_y == 128;
}

JsonObject ProControllerState::serialize_state() const {
    JsonObject obj;
    obj["is_neutral"] = is_neutral();
    obj["buttons"] = button_to_string(buttons);
    obj["dpad"] = dpad_to_string(dpad);
    obj["left_x"] = left_x;
    obj["left_y"] = left_y;
    obj["right_x"] = right_x;
    obj["right_y"] = right_y;
    
    return obj;
}


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






void JoyconState::clear(){
    buttons = BUTTON_NONE;
    joystick_x = 128;
    joystick_y = 128;
}
bool JoyconState::operator==(const ControllerState& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }

    const JoyconState& r = static_cast<const JoyconState&>(x);

    if (buttons != r.buttons){
        return false;
    }
    if (joystick_x != r.joystick_x){
        return false;
    }
    if (joystick_y != r.joystick_y){
        return false;
    }
    return true;
}
bool JoyconState::is_neutral() const{
    return buttons == 0
        && joystick_x == 128
        && joystick_y == 128;
}

JsonObject JoyconState::serialize_state() const {
    JsonObject obj;
    obj["is_neutral"] = is_neutral();
    obj["buttons"] = button_to_string(buttons);
    obj["joystick_x"] = joystick_x;
    obj["joystick_y"] = joystick_y;
    
    return obj;
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

    state.joystick_x = 128;
    state.joystick_y = 128;
    if (joystick_x != 0 || joystick_y != 0){
        neutral = false;
        int mag = std::abs(joystick_x) > std::abs(joystick_y)
            ? std::abs(joystick_x)
            : std::abs(joystick_y);
        state.joystick_x = (uint8_t)std::min(128 * joystick_x / mag + 128, 255);
        state.joystick_y = (uint8_t)std::min(128 * joystick_y / mag + 128, 255);
    }

    return neutral;
}











}
}
