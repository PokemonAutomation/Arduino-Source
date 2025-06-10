/*  Model Type
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ModelType_H
#define PokemonAutomation_NintendoSwitch_ModelType_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


inline const EnumDropdownDatabase<ConsoleType>& CONSOLE_MODEL_DATABASE(){
    static const EnumDropdownDatabase<ConsoleType> database{
       {ConsoleType::Unknown,                       "unknown",                      "Auto-detect if possible."},
       {ConsoleType::Switch1,                       "switch1",                      "Switch 1 + OLED"},
       {ConsoleType::Switch2_Unknown,               "switch2-unknown",              "Switch 2 (unknown model)"},
//       {ConsoleType::Switch2_FW19_International,    "switch2-FW19-international",   "Switch 2 (FW19, international)"},
//       {ConsoleType::Switch2_FW19_JapanLocked,      "switch2-FW19-japan",           "Switch 2 (FW19, Japan-locked)"},
       {ConsoleType::Switch2_FW20_International,    "switch2-FW20-international",   "Switch 2 (FW20, international)"},
       {ConsoleType::Switch2_FW20_JapanLocked,      "switch2-FW20-japan",           "Switch 2 (FW20, Japan-locked)"},
    };
    return database;
}



class ConsoleModelCell : public EnumDropdownCell<ConsoleType>{
public:
    ConsoleModelCell()
        : EnumDropdownCell<ConsoleType>(
            CONSOLE_MODEL_DATABASE(),
            LockMode::LOCK_WHILE_RUNNING,
            ConsoleType::Unknown
        )
    {}

private:
};


class ConsoleModelOption : public EnumDropdownOption<ConsoleType>{
public:
    ConsoleModelOption(std::string label = "<b>Console Type:</b>")
        : EnumDropdownOption<ConsoleType>(
            std::move(label),
            CONSOLE_MODEL_DATABASE(),
            LockMode::LOCK_WHILE_RUNNING,
            ConsoleType::Unknown
        )
    {}

private:
};






}
}
#endif
