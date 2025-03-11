/*  Loto Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_LotoFarmer_H
#define PokemonAutomation_PokemonSwSh_LotoFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class LotoFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    LotoFarmer_Descriptor();
};



class LotoFarmer : public SingleSwitchProgramInstance{
public:
    LotoFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;

    SimpleIntegerOption<uint32_t> SKIPS;
    MillisecondsOption MASH_B_DURATION0;

    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif



