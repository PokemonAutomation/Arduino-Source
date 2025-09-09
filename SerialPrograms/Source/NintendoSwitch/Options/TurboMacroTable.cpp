/*  Turbo Macro Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include <fstream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "NintendoSwitch/Options/TurboMacroTable.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;



const EnumDropdownDatabase<TurboMacroAction>& TurboMacroAction_Database(){
    static const EnumDropdownDatabase<TurboMacroAction> database({
        {TurboMacroAction::NO_ACTION,         "no-action",        "No Action"},
        {TurboMacroAction::LEFT_JOYSTICK,     "left-joystick",    "Left Joystick"},
        {TurboMacroAction::RIGHT_JOYSTICK,    "right-joystick",   "Right Joystick"},
        {TurboMacroAction::LEFT_JOY_CLICK,    "left-joy-click",   "Press Left Joystick (L3)"},
        {TurboMacroAction::RIGHT_JOY_CLICK,   "right-joy-click",  "Press Right Joystick (R3)"},
        {TurboMacroAction::B,                 "button-B",         "Press B"},
        {TurboMacroAction::A,                 "button-A",         "Press A"},
        {TurboMacroAction::Y,                 "button-Y",         "Press Y"},
        {TurboMacroAction::X,                 "button-X",         "Press X"},
        {TurboMacroAction::R,                 "button-R",         "Press R"},
        {TurboMacroAction::L,                 "button-L",         "Press L"},
        {TurboMacroAction::ZR,                "button-ZR",        "Press ZR"},
        {TurboMacroAction::ZL,                "button-ZL",        "Press ZL"},
        {TurboMacroAction::PLUS,              "button-plus",      "Press PLUS"},
        {TurboMacroAction::MINUS,             "button-minus",     "Press MINUS"},
        {TurboMacroAction::DPADLEFT,          "dpad-left",        "Press DPad Left"},
        {TurboMacroAction::DPADRIGHT,         "dpad-right",       "Press DPad Right"},
        {TurboMacroAction::DPADUP,            "dpad-up",          "Press DPad Up"},
        {TurboMacroAction::DPADDOWN,          "dpad-down",        "Press DPad Down"},
        {TurboMacroAction::WAIT,              "wait",             "Wait"}
    });
    return database;
}





TurboMacroCell::~TurboMacroCell(){
    m_action.remove_listener(*this);
}
void TurboMacroCell::operator=(const TurboMacroCell& x){
    x_axis.set(x.x_axis);
    y_axis.set(x.y_axis);
    button_hold.set(x.button_hold.current_text());
    button_release.set(x.button_release.current_text());
    wait.set(x.wait.current_text());
}
TurboMacroCell::TurboMacroCell(EnumDropdownCell<TurboMacroAction>& action)
    : BatchOption(LockMode::LOCK_WHILE_RUNNING, true)
    , m_action(action)
    , x_axis("X:", LockMode::LOCK_WHILE_RUNNING, 128)
    , y_axis("Y:", LockMode::LOCK_WHILE_RUNNING, 128)
    , button_hold(
        "Hold (ms):", false,
        LockMode::LOCK_WHILE_RUNNING,
        0ms, Milliseconds::max(),
        "2000 ms"
    )
    , button_release(
        "Release (ms):", false,
        LockMode::LOCK_WHILE_RUNNING,
        0ms, Milliseconds::max(),
        "2000 ms"
    )
    , wait(
        "Wait (ms):", false,
        LockMode::LOCK_WHILE_RUNNING,
        0ms, Milliseconds::max(),
        "1000 ms"
    )
{
    PA_ADD_OPTION(x_axis);
    PA_ADD_OPTION(y_axis);
    PA_ADD_OPTION(button_hold);
    PA_ADD_OPTION(button_release);
    PA_ADD_OPTION(wait);

    TurboMacroCell::on_config_value_changed(this);
    action.add_listener(*this);
}
void TurboMacroCell::on_config_value_changed(void* object){
    x_axis.set_visibility(ConfigOptionState::HIDDEN);
    y_axis.set_visibility(ConfigOptionState::HIDDEN);
    button_hold.set_visibility(ConfigOptionState::HIDDEN);
    button_release.set_visibility(ConfigOptionState::HIDDEN);
    wait.set_visibility(ConfigOptionState::HIDDEN);
    switch (m_action){
    case TurboMacroAction::LEFT_JOYSTICK:
    case TurboMacroAction::RIGHT_JOYSTICK:
        x_axis.set_visibility(ConfigOptionState::ENABLED);
        y_axis.set_visibility(ConfigOptionState::ENABLED);
    case TurboMacroAction::LEFT_JOY_CLICK:
    case TurboMacroAction::RIGHT_JOY_CLICK:
    case TurboMacroAction::B:
    case TurboMacroAction::A:
    case TurboMacroAction::Y:
    case TurboMacroAction::X:
    case TurboMacroAction::R:
    case TurboMacroAction::L:
    case TurboMacroAction::ZR:
    case TurboMacroAction::ZL:
    case TurboMacroAction::PLUS:
    case TurboMacroAction::MINUS:
    case TurboMacroAction::DPADLEFT:
    case TurboMacroAction::DPADRIGHT:
    case TurboMacroAction::DPADUP:
    case TurboMacroAction::DPADDOWN:
        button_hold.set_visibility(ConfigOptionState::ENABLED);
        button_release.set_visibility(ConfigOptionState::ENABLED);
        break;
    case TurboMacroAction::WAIT:
        wait.set_visibility(ConfigOptionState::ENABLED);
        break;
    default:
        break;
    }
}




TurboMacroRow::TurboMacroRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , action(TurboMacroAction_Database(), LockMode::LOCK_WHILE_RUNNING, TurboMacroAction::NO_ACTION)
    , parameters(action)
{
    PA_ADD_OPTION(action);
    PA_ADD_OPTION(parameters);
}
std::unique_ptr<EditableTableRow> TurboMacroRow::clone() const{
    std::unique_ptr<TurboMacroRow> ret(new TurboMacroRow(parent()));
    ret->action.set(action);
    ret->parameters = parameters;
    return ret;
}

void TurboMacroRow::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    const JsonValue* value = obj->get_value("Action");
    if (value == nullptr){
        return;
    }
    action.load_json(*value);

    switch (action){
    case TurboMacroAction::LEFT_JOYSTICK:
    case TurboMacroAction::RIGHT_JOYSTICK:
        value = obj->get_value("MoveDirectionX");
        if (value != nullptr){
            parameters.x_axis.load_json(*value);
        }
        value = obj->get_value("MoveDirectionY");
        if (value != nullptr){
            parameters.y_axis.load_json(*value);
        }
        value = obj->get_value("Hold");
        if (value != nullptr && value->is_integer()){
            parameters.button_hold.set(std::to_string(value->to_integer_default() * 8));
        }
        value = obj->get_value("HoldMs");
        if (value != nullptr){
            parameters.button_hold.load_json(*value);
        }
        value = obj->get_value("Release");
        if (value != nullptr && value->is_integer()){
            parameters.button_release.set(std::to_string(value->to_integer_default() * 8));
        }
        value = obj->get_value("ReleaseMs");
        if (value != nullptr){
            parameters.button_release.load_json(*value);
        }
        break;
    case TurboMacroAction::LEFT_JOY_CLICK:
    case TurboMacroAction::RIGHT_JOY_CLICK:
    case TurboMacroAction::B:
    case TurboMacroAction::A:
    case TurboMacroAction::Y:
    case TurboMacroAction::X:
    case TurboMacroAction::R:
    case TurboMacroAction::L:
    case TurboMacroAction::ZR:
    case TurboMacroAction::ZL:
    case TurboMacroAction::PLUS:
    case TurboMacroAction::MINUS:
    case TurboMacroAction::DPADLEFT:
    case TurboMacroAction::DPADRIGHT:
    case TurboMacroAction::DPADUP:
    case TurboMacroAction::DPADDOWN:
        value = obj->get_value("Hold");
        if (value != nullptr && value->is_integer()){
            parameters.button_hold.set(std::to_string(value->to_integer_default() * 8));
        }
        value = obj->get_value("HoldMs");
        if (value != nullptr){
            parameters.button_hold.load_json(*value);
        }
        value = obj->get_value("Release");
        if (value != nullptr && value->is_integer()){
            parameters.button_release.set(std::to_string(value->to_integer_default() * 8));
        }
        value = obj->get_value("ReleaseMs");
        if (value != nullptr){
            parameters.button_release.load_json(*value);
        }
        break;
    case TurboMacroAction::WAIT:
        value = obj->get_value("Wait");
        if (value != nullptr && value->is_integer()){
            parameters.wait.set(std::to_string(value->to_integer_default() * 8));
        }
        value = obj->get_value("WaitMs");
        if (value != nullptr){
            parameters.wait.load_json(*value);
        }
        break;
    default:
        break;
    }
}
JsonValue TurboMacroRow::to_json() const{
    JsonObject obj;
    obj["Action"] = action.to_json();
    switch (action){
    case TurboMacroAction::LEFT_JOY_CLICK:
    case TurboMacroAction::RIGHT_JOY_CLICK:
    case TurboMacroAction::B:
    case TurboMacroAction::A:
    case TurboMacroAction::Y:
    case TurboMacroAction::X:
    case TurboMacroAction::R:
    case TurboMacroAction::L:
    case TurboMacroAction::ZR:
    case TurboMacroAction::ZL:
    case TurboMacroAction::PLUS:
    case TurboMacroAction::MINUS:
    case TurboMacroAction::DPADLEFT:
    case TurboMacroAction::DPADRIGHT:
    case TurboMacroAction::DPADUP:
    case TurboMacroAction::DPADDOWN:
        obj["HoldMs"] = parameters.button_hold.to_json();
        obj["ReleaseMs"] = parameters.button_release.to_json();
        break;
    case TurboMacroAction::LEFT_JOYSTICK:
    case TurboMacroAction::RIGHT_JOYSTICK:
        obj["MoveDirectionX"] = parameters.x_axis.to_json();
        obj["MoveDirectionY"] = parameters.y_axis.to_json();
        obj["HoldMs"] = parameters.button_hold.to_json();
        obj["ReleaseMs"] = parameters.button_release.to_json();
        break;
    case TurboMacroAction::WAIT:
        obj["WaitMs"] = parameters.wait.to_json();
        break;
    default:
        break;
    }
    return obj;
}


TurboMacroTable::TurboMacroTable()
    : EditableTableOption_t<TurboMacroRow>(
        "<b>Custom Macro Table:</b><br>"
        "Set a list of button press to create a macro. Joystick direction is specified by (x, y).<br>"
        "x = 0 is left, x = 255 is right. y = 0 is up, y = 255 is down. 128 is neutral for both. Ex.<br>"
        "Move joystick fully left would be (0, 128). Move joystick up-right would be (255, 0).",
        LockMode::LOCK_WHILE_RUNNING
    )
{}
std::vector<std::string> TurboMacroTable::make_header() const{
    return std::vector<std::string>{
        "Action",
        "Parameters",
    };
}










}
}
