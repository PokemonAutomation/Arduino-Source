/*  Test Path Maker Table
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
#include "NintendoSwitch/Options/TestPathMakerTable.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



const EnumDatabase<PathAction>& PathAction_Database(){
    static const EnumDatabase<PathAction> database({
        {PathAction::NO_ACTION,         "no-action",        "No Action"},
        {PathAction::LEFT_JOYSTICK,     "left-joystick",    "Left Joystick"},
        {PathAction::RIGHT_JOYSTICK,    "right-joystick",   "Right Joystick"},
        {PathAction::B,                 "button-B",         "Press B"},
        {PathAction::A,                 "button-A",         "Press A"},
        {PathAction::Y,                 "button-Y",         "Press Y"},
        {PathAction::X,                 "button-X",         "Press X"},
        {PathAction::R,                 "button-R",         "Press R"},
        {PathAction::L,                 "button-L",         "Press L"},
        {PathAction::ZR,                "button-ZR",        "Press ZR"},
        {PathAction::ZL,                "button-ZL",        "Press ZL"},
        {PathAction::PLUS,              "button-plus",      "Press PLUS"},
        {PathAction::MINUS,             "button-minus",     "Press MINUS"},
        {PathAction::DPADLEFT,          "dpad-left",        "Press DPad Left"},
        {PathAction::DPADRIGHT,         "dpad-right",       "Press DPad Right"},
        {PathAction::DPADUP,            "dpad-up",          "Press DPad Up"},
        {PathAction::DPADDOWN,          "dpad-down",        "Press DPad Down"},
        {PathAction::WAIT,              "wait",             "Wait"}
    });
    return database;
}





PathMakerCell::~PathMakerCell(){
    m_action.remove_listener(*this);
}
void PathMakerCell::operator=(const PathMakerCell& x){
    x_axis.set(x.x_axis);
    y_axis.set(x.y_axis);
    button_hold_ticks.set(x.button_hold_ticks);
    button_release_ticks.set(x.button_release_ticks);
    wait_ticks.set(x.wait_ticks);
}
PathMakerCell::PathMakerCell(EnumDropdownCell<PathAction>& action)
    : BatchOption(true)
    , m_action(action)
    , x_axis("X:", 128), y_axis("Y:", 128)
    , button_hold_ticks("Ticks to Hold:", 250)
    , button_release_ticks("Ticks to Release:", 250)
    , wait_ticks("Ticks to Wait:", 125)
{
    PA_ADD_OPTION(x_axis);
    PA_ADD_OPTION(y_axis);
    PA_ADD_OPTION(button_hold_ticks);
    PA_ADD_OPTION(button_release_ticks);
    PA_ADD_OPTION(wait_ticks);

    PathMakerCell::value_changed();
    action.add_listener(*this);
}
void PathMakerCell::value_changed(){
    x_axis.set_visibility(ConfigOptionState::HIDDEN);
    y_axis.set_visibility(ConfigOptionState::HIDDEN);
    button_hold_ticks.set_visibility(ConfigOptionState::HIDDEN);
    button_release_ticks.set_visibility(ConfigOptionState::HIDDEN);
    wait_ticks.set_visibility(ConfigOptionState::HIDDEN);
    switch (m_action){
    case PathAction::LEFT_JOYSTICK:
    case PathAction::RIGHT_JOYSTICK:
    case PathAction::B:
    case PathAction::A:
    case PathAction::Y:
    case PathAction::X:
    case PathAction::R:
    case PathAction::L:
    case PathAction::ZR:
    case PathAction::ZL:
    case PathAction::PLUS:
    case PathAction::MINUS:
    case PathAction::DPADLEFT:
    case PathAction::DPADRIGHT:
    case PathAction::DPADUP:
    case PathAction::DPADDOWN:
        x_axis.set_visibility(ConfigOptionState::ENABLED);
        y_axis.set_visibility(ConfigOptionState::ENABLED);
        button_hold_ticks.set_visibility(ConfigOptionState::ENABLED);
        button_release_ticks.set_visibility(ConfigOptionState::ENABLED);
        break;
    case PathAction::WAIT:
        wait_ticks.set_visibility(ConfigOptionState::ENABLED);
        break;
    default:
        break;
    }
}




PathMakerRow2::PathMakerRow2()
    : action(PathAction_Database(), PathAction::NO_ACTION)
    , parameters(action)
{
    PA_ADD_OPTION(action);
    PA_ADD_OPTION(parameters);
}
std::unique_ptr<EditableTableRow> PathMakerRow2::clone() const{
    std::unique_ptr<PathMakerRow2> ret(new PathMakerRow2());
    ret->action.set(action);
    ret->parameters = parameters;
    return ret;
}

void PathMakerRow2::load_json(const JsonValue& json){
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
    case PathAction::LEFT_JOYSTICK:
    case PathAction::RIGHT_JOYSTICK:
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
    case PathAction::B:
    case PathAction::A:
    case PathAction::Y:
    case PathAction::X:
    case PathAction::R:
    case PathAction::L:
    case PathAction::ZR:
    case PathAction::ZL:
    case PathAction::PLUS:
    case PathAction::MINUS:
    case PathAction::DPADLEFT:
    case PathAction::DPADRIGHT:
    case PathAction::DPADUP:
    case PathAction::DPADDOWN:
        value = obj->get_value("Hold");
        if (value != nullptr){
            parameters.button_hold_ticks.load_json(*value);
        }
        value = obj->get_value("Release");
        if (value != nullptr){
            parameters.button_release_ticks.load_json(*value);
        }
        break;
    case PathAction::WAIT:
        value = obj->get_value("Wait");
        if (value != nullptr){
            parameters.wait_ticks.load_json(*value);
        }
        break;
    default:
        break;
    }
}
JsonValue PathMakerRow2::to_json() const{
    JsonObject obj;
    obj["Action"] = action.to_json();
    switch (action){
    case PathAction::B:
    case PathAction::A:
    case PathAction::Y:
    case PathAction::X:
    case PathAction::R:
    case PathAction::L:
    case PathAction::ZR:
    case PathAction::ZL:
    case PathAction::PLUS:
    case PathAction::MINUS:
    case PathAction::DPADLEFT:
    case PathAction::DPADRIGHT:
    case PathAction::DPADUP:
    case PathAction::DPADDOWN:
        obj["Hold"] = parameters.button_hold_ticks.to_json();
        obj["Release"] = parameters.button_release_ticks.to_json();
        break;
    case PathAction::LEFT_JOYSTICK:
    case PathAction::RIGHT_JOYSTICK:
        obj["MoveDirectionX"] = parameters.x_axis.to_json();
        obj["MoveDirectionY"] = parameters.y_axis.to_json();
        obj["Hold"] = parameters.button_hold_ticks.to_json();
        obj["Release"] = parameters.button_release_ticks.to_json();
        break;
    case PathAction::WAIT:
        obj["Wait"] = parameters.wait_ticks.to_json();
        break;
    default:
        break;
    }
    return obj;
}


PathMakerTable::PathMakerTable()
    : EditableTableOption_t<PathMakerRow2>(
        "<b>Custom Path Table:</b><br>"
        "Set a sequence of actions to navigate the map."
    )
{}
std::vector<std::string> PathMakerTable::make_header() const{
    return std::vector<std::string>{
        "Action",
        "Parameters",
    };
}










}
}
