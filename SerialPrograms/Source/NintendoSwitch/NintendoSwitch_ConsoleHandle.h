/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ConsoleHandle_H
#define PokemonAutomation_NintendoSwitch_ConsoleHandle_H

#include "GameConsole/ConsoleHandle.h"
#include "NintendoSwitch_ConsoleState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class ConsoleHandle : public GameConsole::ConsoleHandle{
public:
    using GameConsole::ConsoleHandle::ConsoleHandle;

    ConsoleState& state(){ return m_console_state; }
    operator ConsoleState&(){ return m_console_state; }


private:
    ConsoleState m_console_state;
};




}
}
#endif


