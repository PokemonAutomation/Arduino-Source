/*  Nintendo Switch Joycon State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch_Joycon.h"
#include "NintendoSwitch_JoyconState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{





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

void JoyconState::load_json(const JsonObject& json){
    clear();

    //  Backwards compatibility.
    if (json.get_boolean_default("is_neutral", false)){
        return;
    }

    {
        std::string buttons_string;
        json.read_string(buttons_string, "buttons");
        buttons = string_to_button(buttons_string);
    }

    //  Backwards compatibility.
    json.read_integer(joystick_x, "joystick_x", 0, 255);
    json.read_integer(joystick_y, "joystick_y", 0, 255);

    json.read_integer(joystick_x, "jx", 0, 255);
    json.read_integer(joystick_y, "jy", 0, 255);
}
JsonObject JoyconState::to_json() const{
    JsonObject obj;
    if (buttons != BUTTON_NONE){
        obj["buttons"] = button_to_string(buttons);
    }
    if (joystick_x != STICK_CENTER || joystick_y != STICK_CENTER){
        obj["jx"] = joystick_x;
        obj["jy"] = joystick_y;
    }
    return obj;
}
void JoyconState::execute(
    CancellableScope& scope,
    AbstractController& controller,
    Milliseconds duration
) const{
    static_cast<JoyconController&>(controller).issue_full_controller_state(
        &scope,
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
        return "pbf_wait(context, " + std::to_string((hold + release).count()) + ");\n";
    }

    std::string hold_str = std::to_string(hold.count()) + "ms";
    std::string release_str = std::to_string(release.count()) + "ms";

    if (non_neutral_fields > 1){
        std::string ret;
        ret += "pbf_controller_state(context, "
            + button_to_code_string(buttons) + ", "
            + std::to_string(joystick_x) + ", " + std::to_string(joystick_y) + ", "
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
        return "pbf_move_joystick(context, "
            + std::to_string(joystick_x) + ", " + std::to_string(joystick_y) + ", "
            + hold_str + ", " + release_str + ");\n";
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Impossible state.");
}















}
}

















