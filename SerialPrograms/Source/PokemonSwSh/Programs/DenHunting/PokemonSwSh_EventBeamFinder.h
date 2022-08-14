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


    void goto_near_den(BotBaseContext& context) const;
    void goto_far_den(BotBaseContext& context) const;
    void drop_wishing_piece(BotBaseContext& context) const;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;

    TimeExpressionOption<uint16_t> WAIT_TIME_IN_DEN;
};


}
}
}
#endif



