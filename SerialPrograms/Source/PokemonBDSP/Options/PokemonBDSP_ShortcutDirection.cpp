/*  Shortcut Direction
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP_ShortcutDirection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const EnumDropdownDatabase<ShortcutDirection>& ShortcutDirection_Nullable(){
    static EnumDropdownDatabase<ShortcutDirection> database({
        {ShortcutDirection::NONE,   "none", "None"},
        {ShortcutDirection::UP,     "up", "Up"},
        {ShortcutDirection::RIGHT,  "right", "Right"},
        {ShortcutDirection::DOWN,   "down", "Down"},
        {ShortcutDirection::LEFT,   "left", "Left"},
    });
    return database;
}
const EnumDropdownDatabase<ShortcutDirection>& ShortcutDirection_Required(){
    static EnumDropdownDatabase<ShortcutDirection> database({
        {ShortcutDirection::UP,     "up", "Up"},
        {ShortcutDirection::RIGHT,  "right", "Right"},
        {ShortcutDirection::DOWN,   "down", "Down"},
        {ShortcutDirection::LEFT,   "left", "Left"},
    });
    return database;
}



ShortcutDirectionOption::ShortcutDirectionOption(std::string label)
    : EnumDropdownOption<ShortcutDirection>(
        std::move(label),
        ShortcutDirection_Nullable(),
        LockMode::LOCK_WHILE_RUNNING,
        ShortcutDirection::NONE
    )
{}

void ShortcutDirectionOption::run(ProControllerContext& context, uint16_t delay){
    uint8_t shortcut_x = 128;
    uint8_t shortcut_y = 128;
    switch (this->get()){
    case ShortcutDirection::NONE:
        pbf_press_button(context, BUTTON_PLUS, 20, 105);
        return;
    case ShortcutDirection::UP:     shortcut_y = 0; break;
    case ShortcutDirection::RIGHT:  shortcut_x = 255; break;
    case ShortcutDirection::DOWN:   shortcut_y = 255; break;
    case ShortcutDirection::LEFT:   shortcut_x = 0; break;
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid shortcut value: " + std::to_string(current_value())
        );
    }

    pbf_mash_button(context, BUTTON_PLUS, 125);
    pbf_move_right_joystick(context, shortcut_x, shortcut_y, 20, delay);
}


}
}
}
