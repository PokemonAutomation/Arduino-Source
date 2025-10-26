/*  Move Box Arrow
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_MoveBoxArrow_H
#define PokemonAutomation_PokemonLZA_MoveBoxArrow_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class MoveBoxArrow_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MoveBoxArrow_Descriptor();
};


class MoveBoxArrow : public SingleSwitchProgramInstance{
public:
    MoveBoxArrow();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint8_t> TARGET_ROW;
    SimpleIntegerOption<uint8_t> TARGET_COL;
};




}
}
}
#endif
