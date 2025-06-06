/*  Model Type
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ModelType_H
#define PokemonAutomation_NintendoSwitch_ModelType_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



enum class ConsoleModel{
    Switch1,
    Switch2_International,
    Switch2_JapanLocked,
};


class ConsoleModelOption : public EnumDropdownOption<ConsoleModel>{
public:
    ConsoleModelOption()
        : EnumDropdownOption<ConsoleModel>(
            std::move("Console Type:"),
            {
               {ConsoleModel::Switch1,                  "switch1",          "Switch 1 + OLED."},
               {ConsoleModel::Switch2_International,    "switch2",          "Switch 2 (international)"},
               {ConsoleModel::Switch2_JapanLocked,      "switch2-japan",    "Switch 2 (Japan-locked)"},
            },
            LockMode::UNLOCK_WHILE_RUNNING,
            ConsoleModel::Switch1
        )
    {}

private:
};






}
}
#endif
