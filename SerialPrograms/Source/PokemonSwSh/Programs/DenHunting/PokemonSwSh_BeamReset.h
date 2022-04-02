/*  Beam Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BeamReset_H
#define PokemonAutomation_PokemonSwSh_BeamReset_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

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

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;

    TimeExpressionOption<uint16_t> DELAY_BEFORE_RESET;
    BooleanCheckBoxOption EXTRA_LINE;
};


}
}
}
#endif



