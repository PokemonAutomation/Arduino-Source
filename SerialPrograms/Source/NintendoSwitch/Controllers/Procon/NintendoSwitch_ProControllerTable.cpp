/*  Nintendo Switch Pro Controller Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Options/CheckboxDropdownDatabase.h"
#include "Common/Cpp/Options/CheckboxDropdownOption.tpp"
#include "NintendoSwitch_ProControllerTable.h"

namespace PokemonAutomation{

template class CheckboxDropdownCell<NintendoSwitch::Button>;

namespace NintendoSwitch{



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
        {Button::BUTTON_HOME,       "Home",     "Home"},
        {Button::BUTTON_CAPTURE,    "Capture",  "Capture"},
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

void register_procon_tables(){
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
            "Action",
        }
    );
}

ProControllerStateRow::~ProControllerStateRow(){
    DURATION.remove_listener(*this);
    BUTTONS.remove_listener(*this);
    DPAD.remove_listener(*this);
    LEFT_JOYSTICK_X.remove_listener(*this);
    LEFT_JOYSTICK_Y.remove_listener(*this);
    RIGHT_JOYSTICK_X.remove_listener(*this);
    RIGHT_JOYSTICK_Y.remove_listener(*this);
}

ProControllerStateRow::ProControllerStateRow(EditableTableOption& parent_table)
    : ControllerStateRow(parent_table)
    , DURATION(LockMode::LOCK_WHILE_RUNNING, "200 ms")
    , BUTTONS(
        "",
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
    , ACTION(false, LockMode::UNLOCK_WHILE_RUNNING, "", "")
{
    PA_ADD_OPTION(DURATION);
    PA_ADD_OPTION(BUTTONS);
    PA_ADD_OPTION(DPAD);
    PA_ADD_OPTION(LEFT_JOYSTICK_X);
    PA_ADD_OPTION(LEFT_JOYSTICK_Y);
    PA_ADD_OPTION(RIGHT_JOYSTICK_X);
    PA_ADD_OPTION(RIGHT_JOYSTICK_Y);
    PA_ADD_OPTION(ACTION);

    ProControllerStateRow::on_config_value_changed(this);
    DURATION.add_listener(*this);
    BUTTONS.add_listener(*this);
    DPAD.add_listener(*this);
    LEFT_JOYSTICK_X.add_listener(*this);
    LEFT_JOYSTICK_Y.add_listener(*this);
    RIGHT_JOYSTICK_X.add_listener(*this);
    RIGHT_JOYSTICK_Y.add_listener(*this);

    // ACTION.set_visibility(ConfigOptionState::DISABLED);
}

std::unique_ptr<EditableTableRow> ProControllerStateRow::clone() const{
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

void ProControllerStateRow::load_json(const JsonValue& json){
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
JsonValue ProControllerStateRow::to_json() const{
    ProControllerState state;
    get_state(state);

    JsonObject json = state.to_json();
    json["ms"] = DURATION.to_json();
    return json;
}

void ProControllerStateRow::get_state(ProControllerState& state) const{
    state.buttons = BUTTONS;
    state.dpad = DPAD;
    state.left_x = LEFT_JOYSTICK_X;
    state.left_y = LEFT_JOYSTICK_Y;
    state.right_x = RIGHT_JOYSTICK_X;
    state.right_y = RIGHT_JOYSTICK_Y;
}
std::unique_ptr<ControllerState> ProControllerStateRow::get_state(Milliseconds& duration) const{
    std::unique_ptr<ProControllerState> ret(new ProControllerState());
    get_state(*ret);
    duration = DURATION;
    return ret;
}

void ProControllerStateRow::on_config_value_changed(void* object){
    ProControllerState state;
    get_state(state);

    ACTION.set(get_controller_action(state));
}


std::string get_controller_action(ProControllerState& state){
    std::string action = "";

    if (state.buttons != BUTTON_NONE){
        action += "Button";
    }
    if (state.dpad != DPAD_NONE){
        if (action != ""){
            action += ", ";
        }
        action += "Dpad";
    }
    if (state.left_x != STICK_CENTER || state.left_y != STICK_CENTER){
        if (action != ""){
            action += ", ";
        }
        action += "L-stick";
        action += " " + get_joystick_direction(state.left_x, state.left_y);
        
    }
    if (state.right_x != STICK_CENTER || state.right_y != STICK_CENTER){
        if (action != ""){
            action += ", ";
        }
        action += "R-stick";
        action += " " + get_joystick_direction(state.right_x, state.right_y);
    }

    if (action == ""){
        return "Wait";
    }

    return action;
}













}
}
