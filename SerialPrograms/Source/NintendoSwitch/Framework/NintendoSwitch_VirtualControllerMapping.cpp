/*  Virtual Controller Mapping
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <vector>
#include <map>
#include <QJsonObject>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/QtJsonTools.h"
#include "NintendoSwitch_VirtualControllerMapping.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


struct ControllerButton_Dpad : public ControllerButton{
    int delta_x;
    int delta_y;
    ControllerButton_Dpad(int x, int y) : delta_x(x), delta_y(y) {}
    virtual void press(VirtualControllerState& state) const override{
        state.dpad_x += delta_x;
        state.dpad_y += delta_y;
    }
    virtual void release(VirtualControllerState& state) const override{
        state.dpad_x -= delta_x;
        state.dpad_y -= delta_y;
    }
};
const ControllerButton_Dpad CONTROLLER_DPAD_UP          (0, -1);
const ControllerButton_Dpad CONTROLLER_DPAD_UPRIGHT     (+1, -1);
const ControllerButton_Dpad CONTROLLER_DPAD_RIGHT       (+1, 0);
const ControllerButton_Dpad CONTROLLER_DPAD_DOWNRIGHT   (+1, +1);
const ControllerButton_Dpad CONTROLLER_DPAD_DOWN        (0, +1);
const ControllerButton_Dpad CONTROLLER_DPAD_DOWNLEFT    (-1, +1);
const ControllerButton_Dpad CONTROLLER_DPAD_LEFT        (-1, 0);
const ControllerButton_Dpad CONTROLLER_DPAD_UPLEFT      (-1, -1);

struct ControllerButton_LeftJoystick : public ControllerButton{
    int delta_x;
    int delta_y;
    ControllerButton_LeftJoystick(int x, int y) : delta_x(x), delta_y(y) {}
    virtual void press(VirtualControllerState& state) const override{
        state.left_joystick_x += delta_x;
        state.left_joystick_y += delta_y;
    }
    virtual void release(VirtualControllerState& state) const override{
        state.left_joystick_x -= delta_x;
        state.left_joystick_y -= delta_y;
    }
};
const ControllerButton_LeftJoystick CONTROLLER_LEFT_JOYSTICK_UP     (0, -1);
const ControllerButton_LeftJoystick CONTROLLER_LEFT_JOYSTICK_RIGHT  (+1, 0);
const ControllerButton_LeftJoystick CONTROLLER_LEFT_JOYSTICK_DOWN   (0, +1);
const ControllerButton_LeftJoystick CONTROLLER_LEFT_JOYSTICK_LEFT   (-1, 0);

struct ControllerButton_RightJoystick : public ControllerButton{
    int delta_x;
    int delta_y;
    ControllerButton_RightJoystick(int x, int y) : delta_x(x), delta_y(y) {}
    virtual void press(VirtualControllerState& state) const override{
        state.right_joystick_x += delta_x;
        state.right_joystick_y += delta_y;
    }
    virtual void release(VirtualControllerState& state) const override{
        state.right_joystick_x -= delta_x;
        state.right_joystick_y -= delta_y;
    }
};
const ControllerButton_RightJoystick CONTROLLER_RIGHT_JOYSTICK_UP   (0, -1);
const ControllerButton_RightJoystick CONTROLLER_RIGHT_JOYSTICK_RIGHT(+1, 0);
const ControllerButton_RightJoystick CONTROLLER_RIGHT_JOYSTICK_DOWN (0, +1);
const ControllerButton_RightJoystick CONTROLLER_RIGHT_JOYSTICK_LEFT (-1, 0);

struct ControllerButton_Button : public ControllerButton{
    Button button;
    ControllerButton_Button(Button x) : button(x) {}
    virtual void press(VirtualControllerState& state) const override{
        state.buttons |= button;
    }
    virtual void release(VirtualControllerState& state) const override{
        state.buttons &= ~button;
    }
};
const ControllerButton_Button CONTROLLER_BUTTON_Y       (BUTTON_Y);
const ControllerButton_Button CONTROLLER_BUTTON_B       (BUTTON_B);
const ControllerButton_Button CONTROLLER_BUTTON_A       (BUTTON_A);
const ControllerButton_Button CONTROLLER_BUTTON_X       (BUTTON_X);
const ControllerButton_Button CONTROLLER_BUTTON_L       (BUTTON_L);
const ControllerButton_Button CONTROLLER_BUTTON_R       (BUTTON_R);
const ControllerButton_Button CONTROLLER_BUTTON_ZL      (BUTTON_ZL);
const ControllerButton_Button CONTROLLER_BUTTON_ZR      (BUTTON_ZR);
const ControllerButton_Button CONTROLLER_BUTTON_MINUS   (BUTTON_MINUS);
const ControllerButton_Button CONTROLLER_BUTTON_PLUS    (BUTTON_PLUS);
const ControllerButton_Button CONTROLLER_BUTTON_LCLICK  (BUTTON_LCLICK);
const ControllerButton_Button CONTROLLER_BUTTON_RCLICK  (BUTTON_RCLICK);
const ControllerButton_Button CONTROLLER_BUTTON_HOME    (BUTTON_HOME);
const ControllerButton_Button CONTROLLER_BUTTON_CAPTURE (BUTTON_CAPTURE);
const ControllerButton_Button CONTROLLER_BUTTON_AR      (BUTTON_A | BUTTON_R);

const std::map<QString, const ControllerButton&>& STRING_TO_BUTTON_MAP(){
    static const std::map<QString, const ControllerButton&> map{
        {"CONTROLLER_DPAD_UP",              CONTROLLER_DPAD_UP},
        {"CONTROLLER_DPAD_UPRIGHT",         CONTROLLER_DPAD_UPRIGHT},
        {"CONTROLLER_DPAD_RIGHT",           CONTROLLER_DPAD_RIGHT},
        {"CONTROLLER_DPAD_DOWNRIGHT",       CONTROLLER_DPAD_DOWNRIGHT},
        {"CONTROLLER_DPAD_DOWN",            CONTROLLER_DPAD_DOWN},
        {"CONTROLLER_DPAD_DOWNLEFT",        CONTROLLER_DPAD_DOWNLEFT},
        {"CONTROLLER_DPAD_LEFT",            CONTROLLER_DPAD_LEFT},
        {"CONTROLLER_DPAD_UPLEFT",          CONTROLLER_DPAD_UPLEFT},

        {"CONTROLLER_LEFT_JOYSTICK_UP",     CONTROLLER_LEFT_JOYSTICK_UP},
        {"CONTROLLER_LEFT_JOYSTICK_RIGHT",  CONTROLLER_LEFT_JOYSTICK_RIGHT},
        {"CONTROLLER_LEFT_JOYSTICK_DOWN",   CONTROLLER_LEFT_JOYSTICK_DOWN},
        {"CONTROLLER_LEFT_JOYSTICK_LEFT",   CONTROLLER_LEFT_JOYSTICK_LEFT},

        {"CONTROLLER_RIGHT_JOYSTICK_UP",    CONTROLLER_RIGHT_JOYSTICK_UP},
        {"CONTROLLER_RIGHT_JOYSTICK_RIGHT", CONTROLLER_RIGHT_JOYSTICK_RIGHT},
        {"CONTROLLER_RIGHT_JOYSTICK_DOWN",  CONTROLLER_RIGHT_JOYSTICK_DOWN},
        {"CONTROLLER_RIGHT_JOYSTICK_LEFT",  CONTROLLER_RIGHT_JOYSTICK_LEFT},

        {"CONTROLLER_BUTTON_Y",             CONTROLLER_BUTTON_Y},
        {"CONTROLLER_BUTTON_B",             CONTROLLER_BUTTON_B},
        {"CONTROLLER_BUTTON_A",             CONTROLLER_BUTTON_A},
        {"CONTROLLER_BUTTON_X",             CONTROLLER_BUTTON_X},

        {"CONTROLLER_BUTTON_L",             CONTROLLER_BUTTON_L},
        {"CONTROLLER_BUTTON_R",             CONTROLLER_BUTTON_R},
        {"CONTROLLER_BUTTON_ZL",            CONTROLLER_BUTTON_ZL},
        {"CONTROLLER_BUTTON_ZR",            CONTROLLER_BUTTON_ZR},

        {"CONTROLLER_BUTTON_MINUS",         CONTROLLER_BUTTON_MINUS},
        {"CONTROLLER_BUTTON_PLUS",          CONTROLLER_BUTTON_PLUS},
        {"CONTROLLER_BUTTON_LCLICK",        CONTROLLER_BUTTON_LCLICK},
        {"CONTROLLER_BUTTON_RCLICK",        CONTROLLER_BUTTON_RCLICK},

        {"CONTROLLER_BUTTON_HOME",          CONTROLLER_BUTTON_HOME},
        {"CONTROLLER_BUTTON_CAPTURE",       CONTROLLER_BUTTON_CAPTURE},

        {"CONTROLLER_BUTTON_AR",            CONTROLLER_BUTTON_AR},
    };
    return map;
}
const ControllerButton* string_to_controller_button(const QString& name){
    const std::map<QString, const ControllerButton&>& map = STRING_TO_BUTTON_MAP();
    auto iter = map.find(name);
    if (iter == map.end()){
        return nullptr;
    }
    return &iter->second;
}
const QString& controller_button_to_string(const ControllerButton& button){
    static std::map<const ControllerButton*, const QString&> map;
    if (map.empty()){
        for (const auto& item : STRING_TO_BUTTON_MAP()){
            map.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(&item.second),
                std::forward_as_tuple(item.first)
            );
        }
    }
    return map.find(&button)->second;
}



std::vector<std::pair<Qt::Key, const ControllerButton&>> keyboard_mapping{
    {Qt::Key::Key_8,            CONTROLLER_DPAD_UP},
    {Qt::Key::Key_9,            CONTROLLER_DPAD_UPRIGHT},
    {Qt::Key::Key_6,            CONTROLLER_DPAD_RIGHT},
    {Qt::Key::Key_3,            CONTROLLER_DPAD_DOWNRIGHT},
    {Qt::Key::Key_2,            CONTROLLER_DPAD_DOWN},
    {Qt::Key::Key_1,            CONTROLLER_DPAD_DOWNLEFT},
    {Qt::Key::Key_4,            CONTROLLER_DPAD_LEFT},
    {Qt::Key::Key_7,            CONTROLLER_DPAD_UPLEFT},

    {Qt::Key::Key_W,            CONTROLLER_LEFT_JOYSTICK_UP},
    {Qt::Key::Key_D,            CONTROLLER_LEFT_JOYSTICK_RIGHT},
    {Qt::Key::Key_S,            CONTROLLER_LEFT_JOYSTICK_DOWN},
    {Qt::Key::Key_A,            CONTROLLER_LEFT_JOYSTICK_LEFT},

    {Qt::Key::Key_Up,           CONTROLLER_RIGHT_JOYSTICK_UP},
    {Qt::Key::Key_Right,        CONTROLLER_RIGHT_JOYSTICK_RIGHT},
    {Qt::Key::Key_Down,         CONTROLLER_RIGHT_JOYSTICK_DOWN},
    {Qt::Key::Key_Left,         CONTROLLER_RIGHT_JOYSTICK_LEFT},

    {Qt::Key::Key_Slash,        CONTROLLER_BUTTON_Y},
    {Qt::Key::Key_Question,     CONTROLLER_BUTTON_Y},
    {Qt::Key::Key_Shift,        CONTROLLER_BUTTON_B},
    {Qt::Key::Key_Control,      CONTROLLER_BUTTON_B},
    {Qt::Key::Key_Enter,        CONTROLLER_BUTTON_A},
    {Qt::Key::Key_Return,       CONTROLLER_BUTTON_A},
    {Qt::Key::Key_Apostrophe,   CONTROLLER_BUTTON_X},
    {Qt::Key::Key_QuoteDbl,     CONTROLLER_BUTTON_X},

    {Qt::Key::Key_Q,            CONTROLLER_BUTTON_L},
    {Qt::Key::Key_E,            CONTROLLER_BUTTON_R},
    {Qt::Key::Key_R,            CONTROLLER_BUTTON_ZL},
    {Qt::Key::Key_Backslash,    CONTROLLER_BUTTON_ZR},
    {Qt::Key::Key_Bar,          CONTROLLER_BUTTON_ZR},

    {Qt::Key::Key_Minus,        CONTROLLER_BUTTON_MINUS},
    {Qt::Key::Key_Underscore,   CONTROLLER_BUTTON_MINUS},
    {Qt::Key::Key_Plus,         CONTROLLER_BUTTON_PLUS},
    {Qt::Key::Key_Equal,        CONTROLLER_BUTTON_PLUS},

    {Qt::Key::Key_C,            CONTROLLER_BUTTON_LCLICK},
    {Qt::Key::Key_0,            CONTROLLER_BUTTON_RCLICK},

    {Qt::Key::Key_Home,         CONTROLLER_BUTTON_HOME},
    {Qt::Key::Key_Escape,       CONTROLLER_BUTTON_HOME},

    {Qt::Key::Key_Insert,       CONTROLLER_BUTTON_CAPTURE},

    {Qt::Key::Key_Y,            CONTROLLER_BUTTON_AR},
};

std::map<Qt::Key, const ControllerButton&> make_keyboard_map(
    const std::vector<std::pair<Qt::Key, const ControllerButton&>> mapping
){
    std::map<Qt::Key, const ControllerButton&> map;
    for (const auto& item : mapping){
        auto iter = map.find(item.first);
        if (iter != map.end()){
            throw ParseException("Duplicate Key: " + std::to_string((uint32_t)item.first));
        }
        map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item.first),
            std::forward_as_tuple(item.second)
        );
    }
    return map;
}

extern std::map<Qt::Key, const ControllerButton&> keyboard_mapping_lookup;
void set_keyboard_mapping(std::vector<std::pair<Qt::Key, const ControllerButton&>> mapping){
    std::map<Qt::Key, const ControllerButton&> map = make_keyboard_map(mapping);
    keyboard_mapping = std::move(mapping);
    keyboard_mapping_lookup = std::move(map);
}
std::map<Qt::Key, const ControllerButton&> keyboard_mapping_lookup = make_keyboard_map(keyboard_mapping);

const ControllerButton* button_lookup(Qt::Key key){
    auto iter = keyboard_mapping_lookup.find(key);
    if (iter == keyboard_mapping_lookup.end()){
        return nullptr;
    }
    return &iter->second;
}



QJsonArray read_keyboard_mapping(){
    QJsonArray array;
    for (const auto& item : keyboard_mapping){
        QJsonObject pair;
        pair.insert("Qt::Key", item.first);
        pair.insert("Button", controller_button_to_string(item.second));
        array.append(pair);
    }
    return array;
}
void set_keyboard_mapping(const QJsonArray& json){
    if (json.isEmpty()){
        return;
    }

    std::vector<std::pair<Qt::Key, const ControllerButton&>> mapping;

    for (const auto& item : json){
        QJsonObject pair = item.toObject();
        int key;
        if (!json_get_int(key, pair, "Qt::Key")){
            continue;
        }
        QString button_name;
        if (!json_get_string(button_name, pair, "Button")){
            continue;
        }
        const ControllerButton* button = string_to_controller_button(button_name);
        if (button == nullptr){
            continue;
        }
        mapping.emplace_back((Qt::Key)key, *button);
    }

    set_keyboard_mapping(std::move(mapping));
}





}
}
