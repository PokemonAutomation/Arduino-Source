/*  Virtual Controller Mapping
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualControllerMapping_H
#define PokemonAutomation_NintendoSwitch_VirtualControllerMapping_H

#include <QJsonArray>
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


struct VirtualControllerState{
    Button buttons = 0;
    int dpad_x = 0;
    int dpad_y = 0;
    int left_joystick_x = 0;
    int left_joystick_y = 0;
    int right_joystick_x = 0;
    int right_joystick_y = 0;

    void print() const;
};


struct ControllerButton{
    virtual void press(VirtualControllerState& state) const = 0;
    virtual void release(VirtualControllerState& state) const = 0;
};

const ControllerButton* string_to_controller_button(const QString& name);
const QString& controller_button_to_string(const ControllerButton& button);

const ControllerButton* button_lookup(Qt::Key key);

QJsonArray read_keyboard_mapping();
void set_keyboard_mapping(const QJsonArray& json);


}
}
#endif
