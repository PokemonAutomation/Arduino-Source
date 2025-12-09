/*  Nintendo Switch Joycon Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Options/CheckboxDropdownDatabase.h"
#include "NintendoSwitch_JoyconTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



const CheckboxDropdownDatabase<Button>& LeftJoycon_Button_Database(){
    static CheckboxDropdownDatabase<Button> database{
        {Button::BUTTON_UP,         "Up",       "\u2191"},
        {Button::BUTTON_RIGHT,      "Right",    "\u2192"},
        {Button::BUTTON_DOWN,       "Down",     "\u2193"},
        {Button::BUTTON_LEFT,       "Left",     "\u2190"},
        {Button::BUTTON_L,          "L",        "L"},
        {Button::BUTTON_ZL,         "ZL",       "ZL"},
        {Button::BUTTON_MINUS,      "-",        "-"},
        {Button::BUTTON_LCLICK,     "L-click",  "L-click"},
        {Button::BUTTON_CAPTURE,    "Capture",  "Capture"},
        {Button::BUTTON_LEFT_SR,    "SR",       "SR"},
        {Button::BUTTON_LEFT_SL,    "SL",       "SL"},
    };
    return database;
}
const CheckboxDropdownDatabase<Button>& RightJoycon_Button_Database(){
    static CheckboxDropdownDatabase<Button> database{
        {Button::BUTTON_Y,          "Y",        "Y"},
        {Button::BUTTON_B,          "B",        "B"},
        {Button::BUTTON_A,          "A",        "A"},
        {Button::BUTTON_X,          "X",        "X"},
        {Button::BUTTON_R,          "R",        "R"},
        {Button::BUTTON_R,          "RL",       "RL"},
        {Button::BUTTON_PLUS,       "+",        "+"},
        {Button::BUTTON_RCLICK,     "R-click",  "R-click"},
        {Button::BUTTON_HOME,       "Home",     "Home"},
        {Button::BUTTON_RIGHT_SR,   "SR",       "SR"},
        {Button::BUTTON_RIGHT_SL,   "SL",       "SL"},
        {Button::BUTTON_C,          "C",        "C (Switch 2)"},
    };
    return database;
}

void register_joycon_tables(){
    ControllerCommandTable::register_controller_type(
        ControllerClass::NintendoSwitch_LeftJoycon,
        ControllerCommandTable::make_row<JoyconStateRow>,
        {
            "Milliseconds",
            "Action",
            "Buttons",
            "Joystick (X)",
            "Joystick (Y)",
        }
    );
    ControllerCommandTable::register_controller_type(
        ControllerClass::NintendoSwitch_RightJoycon,
        ControllerCommandTable::make_row<JoyconStateRow>,
        {
            "Milliseconds",
            "Action",
            "Buttons",
            "Joystick (X)",
            "Joystick (Y)",
        }
    );
}


JoyconStateRow::~JoyconStateRow(){
    DURATION.remove_listener(*this);
    BUTTONS.remove_listener(*this);
    JOYSTICK_X.remove_listener(*this);
    JOYSTICK_Y.remove_listener(*this);
}



JoyconStateRow::JoyconStateRow(EditableTableOption& parent_table)
    : ControllerStateRow(parent_table)
    , DURATION(LockMode::LOCK_WHILE_RUNNING, "200 ms")
    , BUTTONS(
        "",
        static_cast<ControllerCommandTable&>(parent_table).type() == ControllerClass::NintendoSwitch_LeftJoycon
            ? LeftJoycon_Button_Database()
            : RightJoycon_Button_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        BUTTON_NONE
    )
    , JOYSTICK_X(LockMode::UNLOCK_WHILE_RUNNING, 128, 0, 255)
    , JOYSTICK_Y(LockMode::UNLOCK_WHILE_RUNNING, 128, 0, 255)
    , ACTION(false, LockMode::UNLOCK_WHILE_RUNNING, "", "")
{
    PA_ADD_OPTION(DURATION);
    PA_ADD_OPTION(ACTION);
    PA_ADD_OPTION(BUTTONS);
    PA_ADD_OPTION(JOYSTICK_X);
    PA_ADD_OPTION(JOYSTICK_Y);

    JoyconStateRow::on_config_value_changed(this);
    DURATION.add_listener(*this);
    BUTTONS.add_listener(*this);
    JOYSTICK_X.add_listener(*this);
    JOYSTICK_Y.add_listener(*this);
}

std::unique_ptr<EditableTableRow> JoyconStateRow::clone() const{
    std::unique_ptr<JoyconStateRow> ret(new JoyconStateRow(parent()));
    ret->DURATION.set(DURATION.current_text());
    ret->BUTTONS.set_flag(BUTTONS.current_value());
    ret->JOYSTICK_X.set(JOYSTICK_X);
    ret->JOYSTICK_Y.set(JOYSTICK_Y);
    return ret;
}

void JoyconStateRow::load_json(const JsonValue& json){
    const JsonObject& obj = json.to_object_throw();

    do{
        const std::string* duration = obj.get_string("ms");
        if (duration != nullptr){
            DURATION.load_json(*duration);
            break;
        }
        DURATION.set(std::to_string(obj.get_integer_throw("duration_in_ms")));
    }while (false);

    JoyconState state;
    state.load_json(obj);

    BUTTONS.replace_all(state.buttons);
    JOYSTICK_X.set(state.joystick_x);
    JOYSTICK_Y.set(state.joystick_y);
}
JsonValue JoyconStateRow::to_json() const{
    JoyconState state;
    get_state(state);

    JsonObject json = state.to_json();
    json["ms"] = DURATION.to_json();
    return json;
}

void JoyconStateRow::get_state(JoyconState& state) const{
    state.buttons = BUTTONS;
    state.joystick_x = JOYSTICK_X;
    state.joystick_y = JOYSTICK_Y;
}
std::unique_ptr<ControllerState> JoyconStateRow::get_state(Milliseconds& duration) const{
    std::unique_ptr<JoyconState> ret(new JoyconState());
    get_state(*ret);
    duration = DURATION;
    return ret;
}



void JoyconStateRow::on_config_value_changed(void* object){
    JoyconState state;
    get_state(state);

    ACTION.set(get_controller_action(state));
}



std::string get_controller_action(JoyconState& state){
    std::string action = "";

    if (state.buttons != BUTTON_NONE){
        action += "Button";
    }

    if (state.joystick_x != STICK_CENTER || state.joystick_y != STICK_CENTER){
        if (action != ""){
            action += ", ";
        }
        action += "Joystick";
        action += " " + get_joystick_direction(state.joystick_x, state.joystick_y);
        
    }

    if (action == ""){
        return "Wait";
    }

    return action;
}

















}
}
