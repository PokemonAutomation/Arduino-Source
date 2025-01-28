/*  Virtual Controller Mapping
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <vector>
#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch_VirtualControllerMapping.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


#if 0
const ControllerDeltas DELTA_BUTTON_Y       {.buttons = BUTTON_Y};
const ControllerDeltas DELTA_BUTTON_B       {.buttons = BUTTON_B};
const ControllerDeltas DELTA_BUTTON_A       {.buttons = BUTTON_A};
const ControllerDeltas DELTA_BUTTON_X       {.buttons = BUTTON_X};
const ControllerDeltas DELTA_BUTTON_L       {.buttons = BUTTON_L};
const ControllerDeltas DELTA_BUTTON_R       {.buttons = BUTTON_R};
const ControllerDeltas DELTA_BUTTON_ZL      {.buttons = BUTTON_ZL};
const ControllerDeltas DELTA_BUTTON_ZR      {.buttons = BUTTON_ZR};
const ControllerDeltas DELTA_BUTTON_MINUS   {.buttons = BUTTON_MINUS};
const ControllerDeltas DELTA_BUTTON_PLUS    {.buttons = BUTTON_PLUS};
const ControllerDeltas DELTA_BUTTON_LCLICK  {.buttons = BUTTON_LCLICK};
const ControllerDeltas DELTA_BUTTON_RCLICK  {.buttons = BUTTON_RCLICK};
const ControllerDeltas DELTA_BUTTON_HOME    {.buttons = BUTTON_HOME};
const ControllerDeltas DELTA_BUTTON_CAPTURE {.buttons = BUTTON_CAPTURE};
const ControllerDeltas DELTA_BUTTON_AR      {.buttons = BUTTON_A | BUTTON_R};

const ControllerDeltas DELTA_DPAD_UP        {.dpad_x =  0, .dpad_y = -1};
const ControllerDeltas DELTA_DPAD_UPRIGHT   {.dpad_x = +1, .dpad_y = -1};
const ControllerDeltas DELTA_DPAD_RIGHT     {.dpad_x = +1, .dpad_y =  0};
const ControllerDeltas DELTA_DPAD_DOWNRIGHT {.dpad_x = +1, .dpad_y = +1};
const ControllerDeltas DELTA_DPAD_DOWN      {.dpad_x =  0, .dpad_y = +1};
const ControllerDeltas DELTA_DPAD_DOWNLEFT  {.dpad_x = -1, .dpad_y = +1};
const ControllerDeltas DELTA_DPAD_LEFT      {.dpad_x = -1, .dpad_y =  0};
const ControllerDeltas DELTA_DPAD_UPLEFT    {.dpad_x = -1, .dpad_y = -1};

const ControllerDeltas DELTA_LEFT_UP        {.left_x =  0, .left_y = -1};
const ControllerDeltas DELTA_LEFT_RIGHT     {.left_x = +1, .left_y =  0};
const ControllerDeltas DELTA_LEFT_DOWN      {.left_x =  0, .left_y = +1};
const ControllerDeltas DELTA_LEFT_LEFT      {.left_x = -1, .left_y =  0};

const ControllerDeltas DELTA_RIGHT_UP       {.right_x =  0, .right_y = -1};
const ControllerDeltas DELTA_RIGHT_RIGHT    {.right_x = +1, .right_y =  0};
const ControllerDeltas DELTA_RIGHT_DOWN     {.right_x =  0, .right_y = +1};
const ControllerDeltas DELTA_RIGHT_LEFT     {.right_x = -1, .right_y =  0};

std::map<Qt::Key, ControllerDeltas> DELTAS_MAP{
    {Qt::Key::Key_8,            DELTA_DPAD_UP},
    {Qt::Key::Key_9,            DELTA_DPAD_UPRIGHT},
    {Qt::Key::Key_6,            DELTA_DPAD_RIGHT},
    {Qt::Key::Key_3,            DELTA_DPAD_DOWNRIGHT},
    {Qt::Key::Key_2,            DELTA_DPAD_DOWN},
    {Qt::Key::Key_1,            DELTA_DPAD_DOWNLEFT},
    {Qt::Key::Key_4,            DELTA_DPAD_LEFT},
    {Qt::Key::Key_7,            DELTA_DPAD_UPLEFT},

    {Qt::Key::Key_W,            DELTA_LEFT_UP},
    {Qt::Key::Key_D,            DELTA_LEFT_RIGHT},
    {Qt::Key::Key_S,            DELTA_LEFT_DOWN},
    {Qt::Key::Key_A,            DELTA_LEFT_LEFT},

    {Qt::Key::Key_Up,           DELTA_RIGHT_UP},
    {Qt::Key::Key_Right,        DELTA_RIGHT_RIGHT},
    {Qt::Key::Key_Down,         DELTA_RIGHT_DOWN},
    {Qt::Key::Key_Left,         DELTA_RIGHT_LEFT},

    {Qt::Key::Key_Slash,        DELTA_BUTTON_Y},
    {Qt::Key::Key_Question,     DELTA_BUTTON_Y},
    {Qt::Key::Key_Shift,        DELTA_BUTTON_B},
    {Qt::Key::Key_Control,      DELTA_BUTTON_B},
    {Qt::Key::Key_Enter,        DELTA_BUTTON_A},
    {Qt::Key::Key_Return,       DELTA_BUTTON_A},
    {Qt::Key::Key_Apostrophe,   DELTA_BUTTON_X},
    {Qt::Key::Key_QuoteDbl,     DELTA_BUTTON_X},

    {Qt::Key::Key_Q,            DELTA_BUTTON_L},
    {Qt::Key::Key_E,            DELTA_BUTTON_R},
    {Qt::Key::Key_R,            DELTA_BUTTON_ZL},
    {Qt::Key::Key_Backslash,    DELTA_BUTTON_ZR},
    {Qt::Key::Key_Bar,          DELTA_BUTTON_ZR},

    {Qt::Key::Key_Minus,        DELTA_BUTTON_MINUS},
    {Qt::Key::Key_Underscore,   DELTA_BUTTON_MINUS},
    {Qt::Key::Key_Plus,         DELTA_BUTTON_PLUS},
    {Qt::Key::Key_Equal,        DELTA_BUTTON_PLUS},

    {Qt::Key::Key_C,            DELTA_BUTTON_LCLICK},
    {Qt::Key::Key_0,            DELTA_BUTTON_RCLICK},

    {Qt::Key::Key_Home,         DELTA_BUTTON_HOME},
    {Qt::Key::Key_Escape,       DELTA_BUTTON_HOME},
    {Qt::Key::Key_H,            DELTA_BUTTON_HOME},

    {Qt::Key::Key_Insert,       DELTA_BUTTON_CAPTURE},

    {Qt::Key::Key_Y,            DELTA_BUTTON_AR},
};
#endif











#if 0

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
    {Qt::Key::Key_H,            CONTROLLER_BUTTON_HOME},

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



JsonArray read_keyboard_mapping(){
    JsonArray array;
    for (const auto& item : keyboard_mapping){
        JsonObject pair;
        pair["Qt::Key"] = (int64_t)item.first;
        pair["Button"] = controller_button_to_string(item.second).toStdString();
        array.push_back(std::move(pair));
    }
    return array;
}
void set_keyboard_mapping(const JsonArray& json){
    if (json.empty()){
        return;
    }

    std::vector<std::pair<Qt::Key, const ControllerButton&>> mapping;

    for (const auto& item : json){
        const JsonObject* obj = item.to_object();
        if (obj == nullptr){
            continue;
        }
        int key;
        if (!obj->read_integer(key, "Qt::Key")){
            continue;
        }
        const std::string* button_name = obj->get_string("Button");
        if (button_name == nullptr){
            continue;
        }
        const ControllerButton* button = string_to_controller_button(QString::fromStdString(*button_name));
        if (button == nullptr){
            continue;
        }
        mapping.emplace_back((Qt::Key)key, *button);
    }

    set_keyboard_mapping(std::move(mapping));
}

#endif



}
}
