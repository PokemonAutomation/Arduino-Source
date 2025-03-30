/*  Watt Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_WattFarmer_H
#define PokemonAutomation_PokemonSwSh_WattFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class WattFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WattFarmer_Descriptor();
};



class WattFarmer : public SingleSwitchProgramInstance{
public:
    WattFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    MillisecondsOption GRIP_MENU_WAIT0;
    MillisecondsOption EXIT_DEN_WAIT;

    SimpleIntegerOption<uint32_t> SKIPS;
    SimpleIntegerOption<uint16_t> SAVE_ITERATIONS0;

    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif



