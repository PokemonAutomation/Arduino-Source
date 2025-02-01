/*  Virtual Controller State
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Controllers/ControllerSession.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "NintendoSwitch_VirtualControllerState.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;





void SwitchControllerState::clear(){
    buttons = BUTTON_NONE;
    dpad = DPAD_NONE;
    left_x = 128;
    left_y = 128;
    right_x = 128;
    right_y = 128;
}

bool SwitchControllerState::operator==(const ControllerState& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }

    const SwitchControllerState& r = static_cast<const SwitchControllerState&>(x);

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

bool SwitchControllerState::is_neutral() const{
    return buttons == 0
        && dpad == DPAD_NONE
        && left_x == 128
        && left_y == 128
        && right_x == 128
        && right_y == 128;
}
bool SwitchControllerState::send_to_controller(ControllerSession& controller) const{
    controller.logger().log(
        "VirtualController: (" + button_to_string(buttons) +
        "), dpad(" + dpad_to_string(dpad) +
        "), LJ(" + std::to_string(left_x) + "," + std::to_string(left_y) +
        "), RJ(" + std::to_string(right_x) + "," + std::to_string(right_y) +
        ")",
        COLOR_DARKGREEN
    );
    return controller.try_run<SwitchController>([this](SwitchController& controller){
        controller.issue_controller_state(
            nullptr,
            buttons,
            dpad,
            left_x,
            left_y,
            right_x,
            right_y,
            255*8ms
        );
    }).empty();
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
bool ControllerDeltas::to_state(SwitchControllerState& state) const{
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
