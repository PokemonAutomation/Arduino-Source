/*  Virtual Controller Mapping
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_VirtualControllerMapping_H
#define PokemonAutomation_NintendoSwitch_VirtualControllerMapping_H

#include <QKeyEvent>
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NintendoSwitch_VirtualControllerState.h"

class QString;

namespace PokemonAutomation{
    class JsonArray2;
namespace NintendoSwitch{


struct ControllerButton{
    virtual void press(VirtualControllerState& state) const = 0;
    virtual void release(VirtualControllerState& state) const = 0;
};

const ControllerButton* string_to_controller_button(const QString& name);
const QString& controller_button_to_string(const ControllerButton& button);

const ControllerButton* button_lookup(Qt::Key key);

JsonArray2 read_keyboard_mapping();
void set_keyboard_mapping(const JsonArray2& json);



}
}
#endif
