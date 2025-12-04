/*  Nintendo Switch Pro Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/CheckboxDropdownDatabase.h"
#include "Common/Cpp/Options/CheckboxDropdownOption.h"
#include "Common/Cpp/Options/CheckboxDropdownOption.tpp"
#include "Controllers/ControllerStateTable.h"
#include "NintendoSwitch_ProController.h"
#include "NintendoSwitch_ProControllerState.h"

namespace PokemonAutomation{

template class CheckboxDropdownCell<NintendoSwitch::Button>;

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





const CheckboxDropdownDatabase<Button>& ProController_Button_Database(){
    static CheckboxDropdownDatabase<Button> database{
        {Button::BUTTON_Y,          "Y",        "Y"},
        {Button::BUTTON_B,          "B",        "B"},
        {Button::BUTTON_A,          "A",        "A"},
        {Button::BUTTON_X,          "X",        "X"},
        {Button::BUTTON_L,          "L",        "L"},
        {Button::BUTTON_R,          "R",        "R"},
        {Button::BUTTON_ZL,         "ZL",       "ZL"},
        {Button::BUTTON_ZR,         "ZR",       "ZR"},
        {Button::BUTTON_MINUS,      "-",        "-"},
        {Button::BUTTON_PLUS,       "+",        "+"},
        {Button::BUTTON_LCLICK,     "L-click",  "L-click"},
        {Button::BUTTON_RCLICK,     "R-click",  "R-click"},
        {Button::BUTTON_HOME,       "home",     "Home"},
        {Button::BUTTON_CAPTURE,    "capture",  "Capture"},
        {Button::BUTTON_GR,         "GR",       "GR (Switch 2)"},
        {Button::BUTTON_GL,         "GL",       "GL (Switch 2)"},
        {Button::BUTTON_C,          "C",        "C (Switch 2)"},
    };
    return database;
}
const EnumDropdownDatabase<DpadPosition>& ProController_Dpad_Database(){
    static EnumDropdownDatabase<DpadPosition> database{
        {DpadPosition::DPAD_NONE,       "none",         "Dpad: none"},
        {DpadPosition::DPAD_UP,         "up",           "Dpad: \u2191"},
        {DpadPosition::DPAD_UP_RIGHT,   "up-right",     "Dpad: \u2197"},
        {DpadPosition::DPAD_RIGHT,      "right",        "Dpad: \u2192"},
        {DpadPosition::DPAD_DOWN_RIGHT, "down-right",   "Dpad: \u2198"},
        {DpadPosition::DPAD_DOWN,       "down",         "Dpad: \u2193"},
        {DpadPosition::DPAD_DOWN_LEFT,  "down-left",    "Dpad: \u2199"},
        {DpadPosition::DPAD_LEFT,       "left",         "Dpad: \u2190"},
        {DpadPosition::DPAD_UP_LEFT,    "up-left",      "Dpad: \u2196"},
    };
    return database;
}






class ProControllerStateRow : public ControllerStateRow{
public:
    ProControllerStateRow(EditableTableOption& parent_table)
        : ControllerStateRow(parent_table)
        , DURATION(LockMode::LOCK_WHILE_RUNNING, "200 ms")
        , BUTTONS(
            "Buttons",
            ProController_Button_Database(),
            LockMode::UNLOCK_WHILE_RUNNING,
            BUTTON_NONE
        )
        , DPAD(
            ProController_Dpad_Database(),
            LockMode::UNLOCK_WHILE_RUNNING,
            DpadPosition::DPAD_NONE
        )
        , LEFT_JOYSTICK_X(LockMode::UNLOCK_WHILE_RUNNING, 128, 0, 255)
        , LEFT_JOYSTICK_Y(LockMode::UNLOCK_WHILE_RUNNING, 128, 0, 255)
        , RIGHT_JOYSTICK_X(LockMode::UNLOCK_WHILE_RUNNING, 128, 0, 255)
        , RIGHT_JOYSTICK_Y(LockMode::UNLOCK_WHILE_RUNNING, 128, 0, 255)
    {
        PA_ADD_OPTION(DURATION);
        PA_ADD_OPTION(BUTTONS);
        PA_ADD_OPTION(DPAD);
        PA_ADD_OPTION(LEFT_JOYSTICK_X);
        PA_ADD_OPTION(LEFT_JOYSTICK_Y);
        PA_ADD_OPTION(RIGHT_JOYSTICK_X);
        PA_ADD_OPTION(RIGHT_JOYSTICK_Y);
    }

    virtual std::unique_ptr<EditableTableRow> clone() const override{
        std::unique_ptr<ProControllerStateRow> ret(new ProControllerStateRow(parent()));
        ret->DURATION.set(DURATION.current_text());
        ret->BUTTONS.set_flag(BUTTONS.current_value());
        ret->DPAD.set(DPAD);
        ret->LEFT_JOYSTICK_X.set(LEFT_JOYSTICK_X);
        ret->LEFT_JOYSTICK_Y.set(LEFT_JOYSTICK_Y);
        ret->RIGHT_JOYSTICK_X.set(RIGHT_JOYSTICK_X);
        ret->RIGHT_JOYSTICK_Y.set(RIGHT_JOYSTICK_Y);
        return ret;
    }

    virtual void load_json(const JsonValue& json) override{
        const JsonObject& obj = json.to_object_throw();

        do{
            const std::string* duration = obj.get_string("ms");
            if (duration != nullptr){
                DURATION.load_json(*duration);
                break;
            }
            DURATION.set(std::to_string(obj.get_integer_throw("duration_in_ms")));
        }while (false);

        ProControllerState state;
        state.load_json(obj);

        BUTTONS.replace_all(state.buttons);
        DPAD.set(state.dpad);
        LEFT_JOYSTICK_X.set(state.left_x);
        LEFT_JOYSTICK_Y.set(state.left_y);
        RIGHT_JOYSTICK_X.set(state.right_x);
        RIGHT_JOYSTICK_Y.set(state.right_y);
    }
    virtual JsonValue to_json() const override{
        ProControllerState state;
        get_state(state);

        JsonObject json = state.to_json();
        json["duration_in_ms"] = DURATION.to_json();
        return json;
    }

    void get_state(ProControllerState& state) const{
        state.buttons = BUTTONS;
        state.dpad = DPAD;
        state.left_x = LEFT_JOYSTICK_X;
        state.left_y = LEFT_JOYSTICK_Y;
        state.right_x = RIGHT_JOYSTICK_X;
        state.right_y = RIGHT_JOYSTICK_Y;
    }
    virtual std::unique_ptr<ControllerState> get_state(Milliseconds& duration) const override{
        std::unique_ptr<ProControllerState> ret(new ProControllerState());
        get_state(*ret);
        duration = DURATION;
        return ret;
    }

private:
    MillisecondsCell DURATION;
    CheckboxDropdownCell<Button> BUTTONS;
    EnumDropdownCell<DpadPosition> DPAD;
    SimpleIntegerCell<uint8_t> LEFT_JOYSTICK_X;
    SimpleIntegerCell<uint8_t> LEFT_JOYSTICK_Y;
    SimpleIntegerCell<uint8_t> RIGHT_JOYSTICK_X;
    SimpleIntegerCell<uint8_t> RIGHT_JOYSTICK_Y;
};



int initialize_ProController(){
    ControllerCommandTable::register_controller_type(
        ControllerClass::NintendoSwitch_ProController,
        ControllerCommandTable::make_row<ProControllerStateRow>,
        {
            "Milliseconds",
            "Buttons",
            "Dpad",
            "Left JS (X)",
            "Left JS (Y)",
            "Right JS (X)",
            "Right JS (Y)",
        }
    );
    return 0;
}
int init_ProController = initialize_ProController();




}
}
