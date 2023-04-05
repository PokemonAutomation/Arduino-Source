/*  Turbo Macro
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TurboMacro_H
#define PokemonAutomation_NintendoSwitch_TurboMacro_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/TurboMacroTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class TurboMacro_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TurboMacro_Descriptor();
};


class TurboMacro : public SingleSwitchProgramInstance{
public:
    TurboMacro();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void run_macro(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void execute_action(ConsoleHandle& console, BotBaseContext& context, const TurboMacroRow& row);

private:
    TurboMacroTable MACRO;
};



}
}
#endif // PokemonAutomation_NintendoSwitch_TurboMacro_H
