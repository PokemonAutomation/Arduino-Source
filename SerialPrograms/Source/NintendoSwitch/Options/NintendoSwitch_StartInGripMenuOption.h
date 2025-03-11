/*  Start in Grip Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_StartInGripMenu_H
#define PokemonAutomation_NintendoSwitch_StartInGripMenu_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class StartInGripOrGameOption : public IntegerEnumDropdownOption{
public:
    StartInGripOrGameOption(bool grip_menu = false)
        : IntegerEnumDropdownOption(
            "<b>Start Location:</b><br>"
            "If set to start in game, you must disconnect all other controllers.",
            {
                {0, "in-game", "Start in game."},
                {1, "grip-menu", "Start in grip menu."},
            },
            LockMode::LOCK_WHILE_RUNNING,
            grip_menu ? 1 : 0
        )
    {}

    bool start_in_grip_menu() const{
        return current_value() != 0;
    }

};

class StartInGripOrClosedOption : public IntegerEnumDropdownOption{
public:
    StartInGripOrClosedOption(bool grip_menu = false)
        : IntegerEnumDropdownOption(
            "<b>Start Location:</b><br>"
            "If set to start in Switch Home, you must disconnect all other controllers.",
            {
                {0, "home", "Start in Switch Home with game closed."},
                {1, "grip-menu", "Start in grip menu."},
            },
            LockMode::LOCK_WHILE_RUNNING,
            grip_menu ? 1 : 0
        )
    {}

    bool start_in_grip_menu() const{
        return current_value() != 0;
    }

};



}
}
#endif
