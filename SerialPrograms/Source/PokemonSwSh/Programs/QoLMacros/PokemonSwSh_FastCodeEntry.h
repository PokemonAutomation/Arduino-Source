/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FastCodeEntry_H
#define PokemonAutomation_PokemonSwSh_FastCodeEntry_H

#include "CommonFramework/Options/FixedCodeOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class FastCodeEntry_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    FastCodeEntry_Descriptor();
};



class FastCodeEntry : public SingleSwitchProgramInstance{
public:
    FastCodeEntry(const FastCodeEntry_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    FixedCodeOption RAID_CODE;
    TimeExpressionOption<uint16_t> INITIAL_DELAY;
};




}
}
}
#endif



