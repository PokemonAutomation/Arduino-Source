/*  Den Roller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenRoller_H
#define PokemonAutomation_PokemonSwSh_DenRoller_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/Catchability.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DenRoller : public SingleSwitchProgram{
public:
    DenRoller();

    void ring_bell(int count) const;
    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint8_t> SKIPS;
    CatchabilitySelector CATCHABILITY;
    TimeExpression<uint16_t> VIEW_TIME;
};




}
}
}
#endif
