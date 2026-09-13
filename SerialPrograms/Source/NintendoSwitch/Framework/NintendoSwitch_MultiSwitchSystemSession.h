/*  Multi-Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class holds the run-time state for multiple Switch systems.
 *
 *  This class is fully thread-safe. You can call any functions from anywhere at
 *  anytime.
 *
 *  Warning: Constructing this class requires an "option" parameter. It is not
 *  safe to modify this "option" parameter during the lifetime of this class.
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_MultiSwitchSystemSession_H
#define PokemonAutomationn_NintendoSwitch_MultiSwitchSystemSession_H

#include "GameConsole/MultiConsoleSystemSession.h"
#include "NintendoSwitch_SwitchSystemSession.h"
#include "NintendoSwitch_MultiSwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchSystemSession : public GameConsole::MultiConsoleSystemSession{
public:
    MultiSwitchSystemSession(
        MultiSwitchSystemOption& option,
        bool allow_commands_while_locked,
        uint64_t program_id
    )
        : GameConsole::MultiConsoleSystemSession(
            option,
            [=](GameConsole::ConsoleSystemOption& option, size_t console_index){
                return std::make_unique<SwitchSystemSession>(
                    static_cast<SwitchSystemOption&>(option),
                    allow_commands_while_locked,
                    console_index,
                    program_id
                );
            }
        )
    {}

    SwitchSystemSession& operator[](size_t index){
        return static_cast<SwitchSystemSession&>(MultiConsoleSystemSession::operator[](index));
    }
};




}
}
#endif
