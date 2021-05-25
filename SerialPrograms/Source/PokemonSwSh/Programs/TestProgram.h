/*  Test Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TestProgram_H
#define PokemonAutomation_PokemonSwSh_TestProgram_H

#include "CommonFramework/Options/EnumDropdown.h"
#include "CommonFramework/Options/StringSelect.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class TestProgram : public SingleSwitchProgram{
public:
    TestProgram();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    EnumDropdown DROPDOWN;
    StringSelect STRING_SELECT;
};



}
}
}
#endif

