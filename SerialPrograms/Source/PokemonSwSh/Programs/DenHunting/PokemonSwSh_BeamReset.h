/*  Beam Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BeamReset_H
#define PokemonAutomation_PokemonSwSh_BeamReset_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BeamReset_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    BeamReset_Descriptor();
};



class BeamReset : public SingleSwitchProgramInstance{
public:
    BeamReset(const BeamReset_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    TimeExpression<uint16_t> DELAY_BEFORE_RESET;
    BooleanCheckBox EXTRA_LINE;
};


}
}
}
#endif



