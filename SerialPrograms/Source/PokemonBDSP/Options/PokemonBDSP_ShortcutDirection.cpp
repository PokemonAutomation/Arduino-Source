/*  Shortcut Direction
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonBDSP_ShortcutDirection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShortcutDirection::ShortcutDirection(QString label)
    : EnumDropdownOption(
        std::move(label),
        {
            "Up",
            "Right",
            "Down",
            "Left",
        }, 0
    )
{}

void ShortcutDirection::run(const BotBaseContext& context, uint16_t delay){
    uint8_t shortcut_x = 128;
    uint8_t shortcut_y = 128;
    switch (*this){
    case 0: shortcut_y = 0; break;
    case 1: shortcut_x = 255; break;
    case 2: shortcut_y = 255; break;
    case 3: shortcut_x = 0; break;
    default: PA_THROW_StringException("Invalid shortcut value: " + std::to_string(*this));
    }
    pbf_mash_button(context, BUTTON_PLUS, 125);
    pbf_move_right_joystick(context, shortcut_x, shortcut_y, 10, delay);
}


}
}
}
