/*  Turbo Macro
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TurboMacro_H
#define PokemonAutomation_NintendoSwitch_TurboMacro_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Controllers/ControllerStateTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class TurboMacro_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TurboMacro_Descriptor();
};


class TurboMacro : public SingleSwitchProgramInstance{
public:
    TurboMacro();

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;


private:
    SimpleIntegerOption<uint32_t> LOOP;
    ControllerCommandTables TABLE;
};



}
}
#endif // PokemonAutomation_NintendoSwitch_TurboMacro_H
