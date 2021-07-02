/*  Event Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EventBeamFinder_H
#define PokemonAutomation_PokemonSwSh_EventBeamFinder_H

#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EventBeamFinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    EventBeamFinder_Descriptor();
};



class EventBeamFinder : public SingleSwitchProgramInstance{
public:
    EventBeamFinder(const EventBeamFinder_Descriptor& descriptor);


    void goto_near_den(const BotBaseContext& context) const;
    void goto_far_den(const BotBaseContext& context) const;
    void drop_wishing_piece(const BotBaseContext& context) const;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    TimeExpression<uint16_t> WAIT_TIME_IN_DEN;
};


}
}
}
#endif



