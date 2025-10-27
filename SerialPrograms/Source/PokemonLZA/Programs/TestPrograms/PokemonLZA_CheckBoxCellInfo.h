/*  Check Box Cell Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_CheckBoxCellInfo_H
#define PokemonAutomation_PokemonLZA_CheckBoxCellInfo_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class CheckBoxCellInfo_Descriptor : public SingleSwitchProgramDescriptor{
public:
    CheckBoxCellInfo_Descriptor();
};


class CheckBoxCellInfo : public SingleSwitchProgramInstance{
public:
    CheckBoxCellInfo();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
};




}
}
}
#endif
