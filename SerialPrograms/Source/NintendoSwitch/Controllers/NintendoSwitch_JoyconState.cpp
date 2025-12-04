/*  Nintendo Switch Joycon State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/CheckboxDropdownDatabase.h"
#include "Common/Cpp/Options/CheckboxDropdownOption.h"
#include "Common/Cpp/Options/CheckboxDropdownOption.tpp"
#include "Controllers/ControllerStateTable.h"
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
        {Button::BUTTON_CAPTURE,    "capture",  "Capture"},
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
        {Button::BUTTON_HOME,       "home",     "Home"},
        {Button::BUTTON_RIGHT_SR,   "SR",       "SR"},
        {Button::BUTTON_RIGHT_SL,   "SL",       "SL"},
        {Button::BUTTON_C,          "C",        "C (Switch 2)"},
    };
    return database;
}



class JoyconStateRow : public ControllerStateRow{
public:
    JoyconStateRow(EditableTableOption& parent_table)
        : ControllerStateRow(parent_table)
        , DURATION(LockMode::LOCK_WHILE_RUNNING, "200 ms")
        , BUTTONS(
            "Buttons",
            static_cast<ControllerCommandTable&>(parent_table).type() == ControllerClass::NintendoSwitch_LeftJoycon
                ? LeftJoycon_Button_Database()
                : RightJoycon_Button_Database(),
            LockMode::UNLOCK_WHILE_RUNNING,
            BUTTON_NONE
        )
        , JOYSTICK_X(LockMode::UNLOCK_WHILE_RUNNING, 128, 0, 255)
        , JOYSTICK_Y(LockMode::UNLOCK_WHILE_RUNNING, 128, 0, 255)
    {
        PA_ADD_OPTION(DURATION);
        PA_ADD_OPTION(BUTTONS);
        PA_ADD_OPTION(JOYSTICK_X);
        PA_ADD_OPTION(JOYSTICK_Y);
    }

    virtual std::unique_ptr<EditableTableRow> clone() const override{
        std::unique_ptr<JoyconStateRow> ret(new JoyconStateRow(parent()));
        ret->DURATION.set(DURATION.current_text());
        ret->BUTTONS.set_flag(BUTTONS.current_value());
        ret->JOYSTICK_X.set(JOYSTICK_X);
        ret->JOYSTICK_Y.set(JOYSTICK_Y);
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

        JoyconState state;
        state.load_json(obj);

        BUTTONS.replace_all(state.buttons);
        JOYSTICK_X.set(state.joystick_x);
        JOYSTICK_Y.set(state.joystick_y);
    }
    virtual JsonValue to_json() const override{
        JoyconState state;
        get_state(state);

        JsonObject json = state.to_json();
        json["ms"] = DURATION.to_json();
        return json;
    }

    void get_state(JoyconState& state) const{
        state.buttons = BUTTONS;
        state.joystick_x = JOYSTICK_X;
        state.joystick_y = JOYSTICK_Y;
    }
    virtual std::unique_ptr<ControllerState> get_state(Milliseconds& duration) const override{
        std::unique_ptr<JoyconState> ret(new JoyconState());
        get_state(*ret);
        duration = DURATION;
        return ret;
    }

private:
    MillisecondsCell DURATION;
    CheckboxDropdownCell<Button> BUTTONS;
    SimpleIntegerCell<uint8_t> JOYSTICK_X;
    SimpleIntegerCell<uint8_t> JOYSTICK_Y;
};




int initialize_Joycons(){
    ControllerCommandTable::register_controller_type(
        ControllerClass::NintendoSwitch_LeftJoycon,
        ControllerCommandTable::make_row<JoyconStateRow>,
        {
            "Milliseconds",
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
            "Buttons",
            "Joystick (X)",
            "Joystick (Y)",
        }
    );
    return 0;
}
int init_Joycons = initialize_Joycons();


















}
}

















