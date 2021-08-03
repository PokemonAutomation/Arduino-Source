/*  Start in Grip Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_StartInGripMenu_H
#define PokemonAutomation_StartInGripMenu_H

#include "CommonFramework/Options/EnumDropdown.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class StartInGripOrGame : public EnumDropdown{
public:
    StartInGripOrGame(bool grip_menu = true)
        : EnumDropdown(
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
        return EnumDropdown::operator size_t() != 0;
    }

};

class StartInGripOrClosed : public EnumDropdown{
public:
    StartInGripOrClosed(bool grip_menu = true)
        : EnumDropdown(
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
        return EnumDropdown::operator size_t() != 0;
    }

};


}
}
#endif
