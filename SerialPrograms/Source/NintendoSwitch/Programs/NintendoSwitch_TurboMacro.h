/*  Turbo Macro
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TurboMacro_H
#define PokemonAutomation_NintendoSwitch_TurboMacro_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, ControllerContext& context) override;

private:
    void run_macro(SingleSwitchProgramEnvironment& env, ControllerContext& context);

    void execute_action(ConsoleHandle& console, ControllerContext& context, const TurboMacroRow& row);

private:
    SimpleIntegerOption<uint32_t> LOOP;
    TurboMacroTable MACRO;
};



}
}
#endif // PokemonAutomation_NintendoSwitch_TurboMacro_H
