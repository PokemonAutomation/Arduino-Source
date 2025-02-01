/*  Event Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EventBeamFinder_H
#define PokemonAutomation_PokemonSwSh_EventBeamFinder_H

#include "Common/Cpp/Options/TimeExpressionOption.h"
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


    void goto_near_den(SwitchControllerContext& context) const;
    void goto_far_den(SwitchControllerContext& context) const;
    void drop_wishing_piece(SwitchControllerContext& context) const;
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;

    TimeExpressionOption<uint16_t> WAIT_TIME_IN_DEN;
};


}
}
}
#endif



