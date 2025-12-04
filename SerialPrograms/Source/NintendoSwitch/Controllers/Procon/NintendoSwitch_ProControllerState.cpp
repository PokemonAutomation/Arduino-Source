/*  Nintendo Switch Pro Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch_ProController.h"
#include "NintendoSwitch_ProControllerState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




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


void ProControllerState::load_json(const JsonObject& json){
    clear();

    //  Backwards compatibility.
    if (json.get_boolean_default("is_neutral", false)){
        return;
    }

    {
        std::string buttons_string;
        if (json.read_string(buttons_string, "buttons")){
            buttons = string_to_button(buttons_string);
        }
    }
    {
        std::string dpad_string;
        if (json.read_string(dpad_string, "dpad")){
            dpad = string_to_dpad(dpad_string);
        }
    }

    //  Backwards compatibility.
    json.read_integer(left_x, "left_x", 0, 255);
    json.read_integer(left_y, "left_y", 0, 255);
    json.read_integer(right_x, "right_x", 0, 255);
    json.read_integer(right_y, "right_y", 0, 255);

    json.read_integer(left_x, "lx", 0, 255);
    json.read_integer(left_y, "ly", 0, 255);
    json.read_integer(right_x, "rx", 0, 255);
    json.read_integer(right_y, "ry", 0, 255);
}
JsonObject ProControllerState::to_json() const{
    JsonObject obj;
    if (buttons != BUTTON_NONE){
        obj["buttons"] = button_to_string(buttons);
    }
    if (dpad != DPAD_NONE){
        obj["dpad"] = dpad_to_string(dpad);
    }
    if (left_x != STICK_CENTER || left_y != STICK_CENTER){
        obj["lx"] = left_x;
        obj["ly"] = left_y;
    }
    if (right_x != STICK_CENTER || right_y != STICK_CENTER){
        obj["rx"] = right_x;
        obj["ry"] = right_y;
    }
    return obj;
}
void ProControllerState::execute(
    CancellableScope& scope,
    AbstractController& controller,
    Milliseconds duration
) const{
    static_cast<ProController&>(controller).issue_full_controller_state(
        &scope,
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
        return "pbf_wait(context, " + std::to_string((hold + release).count()) + "ms);\n";
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
        if (release != Milliseconds(0)){
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








}
}
