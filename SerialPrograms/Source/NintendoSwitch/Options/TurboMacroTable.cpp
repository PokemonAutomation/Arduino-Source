/*  Turbo Macro Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



const EnumDatabase<TurboMacroAction>& TurboMacroAction_Database(){
    static const EnumDatabase<TurboMacroAction> database({
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
    button_hold_ticks.set(x.button_hold_ticks);
    button_release_ticks.set(x.button_release_ticks);
    wait_ticks.set(x.wait_ticks);
}
TurboMacroCell::TurboMacroCell(EnumDropdownCell<TurboMacroAction>& action)
    : BatchOption(LockWhileRunning::LOCKED, true)
    , m_action(action)
    , x_axis("X:", LockWhileRunning::LOCKED, 128)
    , y_axis("Y:", LockWhileRunning::LOCKED, 128)
    , button_hold_ticks("Ticks to Hold:", LockWhileRunning::LOCKED, 250)
    , button_release_ticks("Ticks to Release:", LockWhileRunning::LOCKED, 250)
    , wait_ticks("Ticks to Wait:", LockWhileRunning::LOCKED, 125)
{
    PA_ADD_OPTION(x_axis);
    PA_ADD_OPTION(y_axis);
    PA_ADD_OPTION(button_hold_ticks);
    PA_ADD_OPTION(button_release_ticks);
    PA_ADD_OPTION(wait_ticks);

    TurboMacroCell::value_changed();
    action.add_listener(*this);
}
void TurboMacroCell::value_changed(){
    x_axis.set_visibility(ConfigOptionState::HIDDEN);
    y_axis.set_visibility(ConfigOptionState::HIDDEN);
    button_hold_ticks.set_visibility(ConfigOptionState::HIDDEN);
    button_release_ticks.set_visibility(ConfigOptionState::HIDDEN);
    wait_ticks.set_visibility(ConfigOptionState::HIDDEN);
    switch (m_action){
    case TurboMacroAction::LEFT_JOYSTICK:
    case TurboMacroAction::RIGHT_JOYSTICK:
        x_axis.set_visibility(ConfigOptionState::ENABLED);
        y_axis.set_visibility(ConfigOptionState::ENABLED);
    case TurboMacroAction::B:
    case TurboMacroAction::LEFT_JOY_CLICK:
    case TurboMacroAction::RIGHT_JOY_CLICK:
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
        button_hold_ticks.set_visibility(ConfigOptionState::ENABLED);
        button_release_ticks.set_visibility(ConfigOptionState::ENABLED);
        break;
    case TurboMacroAction::WAIT:
        wait_ticks.set_visibility(ConfigOptionState::ENABLED);
        break;
    default:
        break;
    }
}




TurboMacroRow::TurboMacroRow()
    : action(TurboMacroAction_Database(), LockWhileRunning::LOCKED, TurboMacroAction::NO_ACTION)
    , parameters(action)
{
    PA_ADD_OPTION(action);
    PA_ADD_OPTION(parameters);
}
std::unique_ptr<EditableTableRow> TurboMacroRow::clone() const{
    std::unique_ptr<TurboMacroRow> ret(new TurboMacroRow());
    ret->action.set(action);
    ret->parameters = parameters;
    return ret;
}

void TurboMacroRow::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
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
        if (value != nullptr){
            parameters.button_hold_ticks.load_json(*value);
        }
        value = obj->get_value("Release");
        if (value != nullptr){
            parameters.button_release_ticks.load_json(*value);
        }
        break;
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
        if (value != nullptr){
            parameters.button_hold_ticks.load_json(*value);
        }
        value = obj->get_value("Release");
        if (value != nullptr){
            parameters.button_release_ticks.load_json(*value);
        }
        break;
    case TurboMacroAction::WAIT:
        value = obj->get_value("Wait");
        if (value != nullptr){
            parameters.wait_ticks.load_json(*value);
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
        obj["Hold"] = parameters.button_hold_ticks.to_json();
        obj["Release"] = parameters.button_release_ticks.to_json();
        break;
    case TurboMacroAction::LEFT_JOYSTICK:
    case TurboMacroAction::RIGHT_JOYSTICK:
        obj["MoveDirectionX"] = parameters.x_axis.to_json();
        obj["MoveDirectionY"] = parameters.y_axis.to_json();
        obj["Hold"] = parameters.button_hold_ticks.to_json();
        obj["Release"] = parameters.button_release_ticks.to_json();
        break;
    case TurboMacroAction::WAIT:
        obj["Wait"] = parameters.wait_ticks.to_json();
        break;
    default:
        break;
    }
    return obj;
}


TurboMacroTable::TurboMacroTable()
    : EditableTableOption_t<TurboMacroRow>(
        "<b>Custom Macro Table:</b><br>"
        "Set a list of button press to create a macro. 125 ticks = 1 second. Joystick direction is specified by (x, y).<br>"
        "x = 0 is left, x = 255 is right. y = 0 is up, y = 255 is down. 128 is neutral for both. Ex. Move joystick fully left would be (0, 128). Move joystick up-right would be (255, 0).",
        LockWhileRunning::LOCKED
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
