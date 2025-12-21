/*  Nintendo Switch Pro Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Controllers/JoystickTools.h"
#include "NintendoSwitch_ProController.h"
#include "NintendoSwitch_ProControllerState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




void ProControllerState::clear(){
    buttons = BUTTON_NONE;
    dpad = DPAD_NONE;
    left_joystick = JoystickPosition();
    right_joystick = JoystickPosition();
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
    if (left_joystick != r.left_joystick){
        return false;
    }
    if (right_joystick != r.right_joystick){
        return false;
    }
    return true;
}
bool ProControllerState::is_neutral() const{
    return buttons == 0
        && dpad == DPAD_NONE
        && left_joystick.is_neutral()
        && right_joystick.is_neutral();
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
    {
        uint8_t left_x = 128;
        uint8_t left_y = 128;
        uint8_t right_x = 128;
        uint8_t right_y = 128;

        json.read_integer(left_x, "left_x", 0, 255);
        json.read_integer(left_y, "left_y", 0, 255);
        json.read_integer(right_x, "right_x", 0, 255);
        json.read_integer(right_y, "right_y", 0, 255);

        json.read_integer(left_x, "lx", 0, 255);
        json.read_integer(left_y, "ly", 0, 255);
        json.read_integer(right_x, "rx", 0, 255);
        json.read_integer(right_y, "ry", 0, 255);

        left_joystick.x = JoystickTools::linear_u8_to_float(left_x);
        left_joystick.y = -JoystickTools::linear_u8_to_float(left_y);
        right_joystick.x = JoystickTools::linear_u8_to_float(right_x);
        right_joystick.y = -JoystickTools::linear_u8_to_float(right_y);
    }

    json.read_float(left_joystick.x, "lxf");
    json.read_float(left_joystick.y, "lyf");
    json.read_float(right_joystick.x, "rxf");
    json.read_float(right_joystick.y, "ryf");
}
JsonObject ProControllerState::to_json() const{
    JsonObject obj;
    if (buttons != BUTTON_NONE){
        obj["buttons"] = button_to_string(buttons);
    }
    if (dpad != DPAD_NONE){
        obj["dpad"] = dpad_to_string(dpad);
    }
    if (!left_joystick.is_neutral()){
        obj["lxf"] = left_joystick.x;
        obj["lyf"] = left_joystick.y;
    }
    if (!right_joystick.is_neutral()){
        obj["rxf"] = right_joystick.x;
        obj["ryf"] = right_joystick.y;
    }
    return obj;
}
void ProControllerState::execute(
    Cancellable* scope,
    bool enable_logging,
    AbstractController& controller,
    Milliseconds duration
) const{
    controller.cast_with_exception<ProController>().issue_full_controller_state(
        scope,
        enable_logging,
        duration,
        buttons,
        dpad,
        left_joystick,
        right_joystick
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
        if (!left_joystick.is_neutral()){
            non_neutral_field = 2;
            non_neutral_fields++;
        }
        if (!right_joystick.is_neutral()){
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
            + dpad_to_code_string(dpad) + ", {"
            + tostr_fixed(left_joystick.x, 3) + ", " + tostr_fixed(left_joystick.y, 3) + "}, {"
            + tostr_fixed(right_joystick.x, 3) + ", " + tostr_fixed(right_joystick.y, 3) + "}, "
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
        return "pbf_move_left_joystick(context, {"
            + std::to_string(left_joystick.x) + ", " + std::to_string(left_joystick.y) + "}, "
            + hold_str + ", " + release_str + ");\n";
    case 3:
        return "pbf_move_right_joystick(context, {"
            + std::to_string(right_joystick.x) + ", " + std::to_string(right_joystick.y) + "}, "
            + hold_str + ", " + release_str + ");\n";
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Impossible state.");
}








}
}
