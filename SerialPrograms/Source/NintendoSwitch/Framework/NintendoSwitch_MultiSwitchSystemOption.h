/*  Multi-Switch System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents the serializable state of a set of Switch.
 *  consoles. Specifically, it holds a SwitchSystemOption for each of the
 *  consoles it represents.
 *
 *  This class maintains no runtime state or UI and is not thread-safe.
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_MultiSwitchSystemOption_H
#define PokemonAutomationn_NintendoSwitch_MultiSwitchSystemOption_H

#include "GameConsole/MultiConsoleSystemOption.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchSystemOption final : public GameConsole::MultiConsoleSystemOption{
public:
    MultiSwitchSystemOption(
        size_t min_consoles,
        size_t max_consoles,
        size_t consoles
    )
        : MultiConsoleSystemOption(
            min_consoles,
            max_consoles,
            consoles,
            []{ return std::make_unique<SwitchSystemOption>(); }
        )
    {}
    MultiSwitchSystemOption(
        size_t min_consoles,
        size_t max_consoles,
        const JsonValue& json
    )
        : MultiConsoleSystemOption(
            min_consoles,
            max_consoles,
            json,
            []{ return std::make_unique<SwitchSystemOption>(); }
        )
    {}

    SwitchSystemOption& operator[](size_t index){
        return static_cast<SwitchSystemOption&>(MultiConsoleSystemOption::operator[](index));
    }
};




}
}
#endif
