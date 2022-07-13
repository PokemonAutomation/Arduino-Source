/*  Max Lair Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
    : EnumDropdownOption(
        "<b>Host Switch:</b><br>This is the Switch that hosts the raid.",
        {
            "Switch 0 (Top Left)",
            "Switch 1 (Top Right)",
            "Switch 2 (Bottom Left)",
            "Switch 3 (Bottom Right)",
        },
        0
    )
{}
std::string HostingSwitch::check_validity(size_t consoles) const{
    if (*this >= consoles){
        return "Host Switch cannot be larger than " + std::to_string(consoles - 1) +
        " since you only have " + std::to_string(consoles) + " Switch(es) enabled.";
    }
    return std::string();
}

BossSlot::BossSlot()
    : EnumDropdownOption(
        "<b>Boss Slot:</b>",
        {
            "Anything is fine",
            "Slot 1",
            "Slot 2",
            "Slot 3",
        },
        0
    )
{}




}
}
}
}
