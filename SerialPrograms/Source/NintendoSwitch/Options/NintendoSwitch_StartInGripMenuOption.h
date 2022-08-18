/*  Start in Grip Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_StartInGripMenu_H
#define PokemonAutomation_NintendoSwitch_StartInGripMenu_H

#include "Common/Cpp/Options/DropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class StartInGripOrGameOption : public DropdownOption{
public:
    StartInGripOrGameOption(bool grip_menu = false)
        : DropdownOption(
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
        return current_index() != 0;
    }

};

class StartInGripOrClosedOption : public DropdownOption{
public:
    StartInGripOrClosedOption(bool grip_menu = true)
        : DropdownOption(
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
        return current_index() != 0;
    }

};


}
}
#endif
