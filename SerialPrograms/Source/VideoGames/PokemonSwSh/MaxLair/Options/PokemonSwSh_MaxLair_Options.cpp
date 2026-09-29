/*  Max Lair Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh_MaxLair_Options.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



const std::string MODULE_NAME = "Max Lair";
const std::chrono::milliseconds INFERENCE_RATE = std::chrono::milliseconds(200);




HostingSwitch::HostingSwitch()
    : IntegerEnumDropdownOption(
        "<b>Host Switch:</b><br>This is the Switch that hosts the raid.",
        {
            {0, "switch0", "Switch 0 (Top Left)"},
            {1, "switch1", "Switch 1 (Top Right)"},
            {2, "switch2", "Switch 2 (Bottom Left)"},
            {3, "switch3", "Switch 3 (Bottom Right)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
{}
std::string HostingSwitch::check_validity(size_t consoles) const{
    if (current_value() >= consoles){
        return "Host Switch cannot be larger than " + std::to_string(consoles - 1) +
        " since you only have " + std::to_string(consoles) + " Switch(es) enabled.";
    }
    return std::string();
}

BossSlotOption::BossSlotOption()
    : IntegerEnumDropdownOption(
        "<b>Boss Slot:</b>",
        {
            {0, "anything", "Anything is fine"},
            {1, "slot1", "Slot 1"},
            {2, "slot2", "Slot 2"},
            {3, "slot3", "Slot 3"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
{}




}
}
}
}
