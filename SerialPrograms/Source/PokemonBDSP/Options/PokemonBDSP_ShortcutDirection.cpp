/*  Shortcut Direction
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP_ShortcutDirection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

std::vector<std::string> buildShortcutValues(bool required){
    std::vector<std::string> values;
    if (!required){
        values.emplace_back("None");
    }
    values.emplace_back("Up");
    values.emplace_back("Right");
    values.emplace_back("Down");
    values.emplace_back("Left");
    return values;
}


ShortcutDirection::ShortcutDirection(std::string label, bool required)
    : DropdownOption(
        std::move(label),
        buildShortcutValues(required),
        0
    )
    , m_required(required)
{}

void ShortcutDirection::run(BotBaseContext& context, uint16_t delay){
    uint8_t shortcut_x = 128;
    uint8_t shortcut_y = 128;
    size_t index = *this;
    if (!m_required){
        // If the shortcut direction is optional, the first value is "None".
        if (index == 0) {
            return;
        }
        --index;
    }

    switch (index){
    case 0: shortcut_y = 0; break;
    case 1: shortcut_x = 255; break;
    case 2: shortcut_y = 255; break;
    case 3: shortcut_x = 0; break;
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid shortcut value: " + std::to_string(*this)
        );
    }

    pbf_mash_button(context, BUTTON_PLUS, 125);
    pbf_move_right_joystick(context, shortcut_x, shortcut_y, 20, delay);
}


}
}
}
