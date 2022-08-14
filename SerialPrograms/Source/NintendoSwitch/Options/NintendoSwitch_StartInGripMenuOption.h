/*  Start in Grip Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_StartInGripMenu_H
#define PokemonAutomation_NintendoSwitch_StartInGripMenu_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class StartInGripOrGameOption : public EnumDropdownOption{
public:
    StartInGripOrGameOption(bool grip_menu = false)
        : EnumDropdownOption(
            "<b>Start Location:</b><br>"
            "If set to start in game, you must disconnect all other controllers.",
            {
                "Start in game.",
                "Start in grip menu.",
            },
            grip_menu ? 1 : 0
        )
    {}

    operator bool() const{
        return EnumDropdownOption::operator size_t() != 0;
    }

};

class StartInGripOrClosedOption : public EnumDropdownOption{
public:
    StartInGripOrClosedOption(bool grip_menu = true)
        : EnumDropdownOption(
            "<b>Start Location:</b><br>"
            "If set to start in Switch Home, you must disconnect all other controllers.",
            {
                "Start in Switch Home with game closed.",
                "Start in grip menu.",
            },
            grip_menu ? 1 : 0
        )
    {}

    operator bool() const{
        return EnumDropdownOption::operator size_t() != 0;
    }

};


}
}
#endif
