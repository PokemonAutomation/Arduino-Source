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


const EnumDatabase<ShortcutDirection>& ShortcutDirection_Nullable(){
    static EnumDatabase<ShortcutDirection> database({
        {ShortcutDirection::NONE,   "none", "None"},
        {ShortcutDirection::UP,     "up", "Up"},
        {ShortcutDirection::RIGHT,  "right", "Right"},
        {ShortcutDirection::DOWN,   "down", "Down"},
        {ShortcutDirection::LEFT,   "left", "Left"},
    });
    return database;
}
const EnumDatabase<ShortcutDirection>& ShortcutDirection_Required(){
    static EnumDatabase<ShortcutDirection> database({
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
        false ? ShortcutDirection_Required() : ShortcutDirection_Nullable(),
        ShortcutDirection::NONE
    )
{}

void ShortcutDirectionOption::run(BotBaseContext& context, uint16_t delay){
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
