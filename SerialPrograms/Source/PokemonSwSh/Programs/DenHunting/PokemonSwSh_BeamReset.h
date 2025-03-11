/*  Beam Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BeamReset_H
#define PokemonAutomation_PokemonSwSh_BeamReset_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BeamReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BeamReset_Descriptor();
};



class BeamReset : public SingleSwitchProgramInstance{
public:
    BeamReset();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;

    MillisecondsOption DELAY_BEFORE_RESET0;
    BooleanCheckBoxOption EXTRA_LINE;
};


}
}
}
#endif



