/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FastCodeEntry_H
#define PokemonAutomation_PokemonSwSh_FastCodeEntry_H

#include "Common/Cpp/Options/FixedCodeOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class FastCodeEntry_Descriptor : public SingleSwitchProgramDescriptor{
public:
    FastCodeEntry_Descriptor();
};



class FastCodeEntry : public SingleSwitchProgramInstance{
public:
    FastCodeEntry();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    FixedCodeOption RAID_CODE;
    MillisecondsOption INITIAL_DELAY0;
};




}
}
}
#endif



