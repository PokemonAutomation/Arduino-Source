/*  Virtual Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch_ProController.h"
#include "NintendoSwitch_Joycon.h"
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


void ProControllerState::load_json(const JsonValue& json){
    clear();

    if (json.is_null()){
        return;
    }

    const JsonObject& obj = json.to_object_throw();

    //  Backwards compatibility.
    if (obj.get_boolean_default("is_neutral", false)){
        return;
    }

    {
        std::string buttons_string;
        obj.read_string(buttons_string, "buttons");
        buttons = string_to_button(buttons_string);
    }
    {
        std::string dpad_string;
        obj.read_string(dpad_string, "dpad");
        dpad = string_to_dpad(dpad_string);
    }

    //  Backwards compatibility.
    obj.read_integer(left_x, "left_x", 0, 255);
    obj.read_integer(left_y, "left_y", 0, 255);
    obj.read_integer(right_x, "right_x", 0, 255);
    obj.read_integer(right_y, "right_y", 0, 255);

    obj.read_integer(left_x, "lx", 0, 255);
    obj.read_integer(left_y, "ly", 0, 255);
    obj.read_integer(right_x, "rx", 0, 255);
    obj.read_integer(right_y, "ry", 0, 255);
}
JsonValue ProControllerState::to_json() const{
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
void ProControllerState::execute(AbstractControllerContext& context, Milliseconds duration) const{
    ProController& controller = static_cast<ProController&>(context.controller());
    controller.issue_full_controller_state(
        &context,
        true,
        duration,
        buttons,
        dpad,
        left_x, left_y,
        right_x, right_y
    );
}
std::string ProControllerState::to_cpp(Milliseconds hold, Milliseconds release) const{
    uint8_t non_neutral_field = 0;
    size_t non_neutral_fields = 0;
    do{
        if (buttons != BUTTON_NONE){
            non_neutral_field = 0;
            non_neutral_fields++;
        }
        if (dpad != DPAD_NONE){
            non_neutral_field = 1;
            non_neutral_fields++;
        }
        if (left_x != STICK_CENTER || left_y != STICK_CENTER){
            non_neutral_field = 2;
            non_neutral_fields++;
        }
        if (right_x != STICK_CENTER || right_y != STICK_CENTER){
            non_neutral_field = 3;
            non_neutral_fields++;
        }
    }while (false);

    if (non_neutral_fields == 0){
        return "pbf_wait(context, " + std::to_string((hold + release).count()) + "ms);";
    }

    std::string hold_str = std::to_string(hold.count()) + "ms";
    std::string release_str = std::to_string(release.count()) + "ms";

    if (non_neutral_fields > 1){
        std::string ret;
        ret += "pbf_controller_state(context, "
            + button_to_code_string(buttons) + ", "
            + dpad_to_code_string(dpad) + ", "
            + std::to_string(left_x) + ", " + std::to_string(left_y) + ", "
            + std::to_string(right_x) + ", " + std::to_string(right_y) + ", "
            + hold_str +");\n";
        if (release != 0ms){
            ret += "pbf_wait(context, " + release_str + ");\n";
        }
        return ret;
    }
    switch (non_neutral_field){
    case 0:
        return "pbf_press_button(context, "
            + button_to_code_string(buttons) + ", "
            + hold_str + ", " + release_str + ");\n";
    case 1:
        return "pbf_press_dpad(context, "
            + dpad_to_code_string(dpad) + ", "
            + hold_str + ", " + release_str + ");\n";
    case 2:
        return "pbf_move_left_joystick(context, "
            + std::to_string(left_x) + ", " + std::to_string(left_y) + ", "
            + hold_str + ", " + release_str + ");\n";
    case 3:
        return "pbf_move_right_joystick(context, "
            + std::to_string(right_x) + ", " + std::to_string(right_y) + ", "
            + hold_str + ", " + release_str + ");\n";
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Impossible state.");
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

void JoyconState::load_json(const JsonValue& json){
    clear();

    if (json.is_null()){
        return;
    }

    const JsonObject& obj = json.to_object_throw();

    //  Backwards compatibility.
    if (obj.get_boolean_default("is_neutral", false)){
        return;
    }

    {
        std::string buttons_string;
        obj.read_string(buttons_string, "buttons");
        buttons = string_to_button(buttons_string);
    }

    //  Backwards compatibility.
    obj.read_integer(joystick_x, "joystick_x", 0, 255);
    obj.read_integer(joystick_y, "joystick_y", 0, 255);

    obj.read_integer(joystick_x, "jx", 0, 255);
    obj.read_integer(joystick_y, "jy", 0, 255);
}
JsonValue JoyconState::to_json() const{
    JsonObject obj;
    obj["is_neutral"] = is_neutral();
    obj["buttons"] = button_to_string(buttons);
    obj["joystick_x"] = joystick_x;
    obj["joystick_y"] = joystick_y;
    return obj;
}
void JoyconState::execute(AbstractControllerContext& context, Milliseconds duration) const{
    JoyconController& controller = static_cast<JoyconController&>(context.controller());
    controller.issue_full_controller_state(
        &context,
        true,
        duration,
        buttons,
        joystick_x, joystick_y
    );
}
std::string JoyconState::to_cpp(Milliseconds hold, Milliseconds release) const{
    uint8_t non_neutral_field = 0;
    size_t non_neutral_fields = 0;
    do{
        if (buttons != BUTTON_NONE){
            non_neutral_field = 0;
            non_neutral_fields++;
        }
        if (joystick_x != STICK_CENTER || joystick_y != STICK_CENTER){
            non_neutral_field = 1;
            non_neutral_fields++;
        }
    }while (false);

    if (non_neutral_fields == 0){
        return "pbf_wait(context, " + std::to_string((hold + release).count()) + ");";
    }

    std::string hold_str = std::to_string(hold.count()) + "ms";
    std::string release_str = std::to_string(release.count()) + "ms";

    if (non_neutral_fields > 1){
        std::string ret;
        ret += "pbf_controller_state(context, "
            + button_to_code_string(buttons) + ", "
            + std::to_string(joystick_x) + ", " + std::to_string(joystick_y) + ", "
            + hold_str +");\n";
        if (release != 0ms){
            ret += "pbf_wait(context, " + release_str + ");\n";
        }
        return ret;
    }
    switch (non_neutral_field){
    case 0:
        return "pbf_press_button(context, "
            + button_to_code_string(buttons) + ", "
            + hold_str + ", " + release_str + ");\n";
    case 1:
        return "pbf_move_joystick(context, "
            + std::to_string(joystick_x) + ", " + std::to_string(joystick_y) + ", "
            + hold_str + ", " + release_str + ");\n";
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Impossible state.");
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
