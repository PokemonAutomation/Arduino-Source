/*  Test Box Cell Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_TestBoxCellInfo_H
#define PokemonAutomation_PokemonLZA_TestBoxCellInfo_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class TestBoxCellInfo_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TestBoxCellInfo_Descriptor();
};


class TestBoxCellInfo : public SingleSwitchProgramInstance{
public:
    TestBoxCellInfo();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
};




}
}
}
#endif
