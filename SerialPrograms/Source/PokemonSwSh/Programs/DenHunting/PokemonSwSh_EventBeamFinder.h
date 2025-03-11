/*  Event Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EventBeamFinder_H
#define PokemonAutomation_PokemonSwSh_EventBeamFinder_H

#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EventBeamFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EventBeamFinder_Descriptor();
};



class EventBeamFinder : public SingleSwitchProgramInstance{
public:
    EventBeamFinder();


    void goto_near_den(ProControllerContext& context) const;
    void goto_far_den(ProControllerContext& context) const;
    void drop_wishing_piece(ProControllerContext& context) const;
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;

    MillisecondsOption WAIT_TIME_IN_DEN0;
};


}
}
}
#endif



