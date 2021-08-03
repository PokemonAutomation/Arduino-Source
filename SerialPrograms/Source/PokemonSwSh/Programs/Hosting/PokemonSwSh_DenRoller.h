/*  Den Roller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenRoller_H
#define PokemonAutomation_PokemonSwSh_DenRoller_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/Catchability.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DenRoller_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    DenRoller_Descriptor();
};



class DenRoller : public SingleSwitchProgramInstance{
public:
    DenRoller(const DenRoller_Descriptor& descriptor);

    void ring_bell(const BotBaseContext& context, int count) const;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    SimpleInteger<uint8_t> SKIPS;
    CatchabilitySelector CATCHABILITY;
    TimeExpression<uint16_t> VIEW_TIME;
};




}
}
}
#endif
