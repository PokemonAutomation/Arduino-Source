/*  Berry Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BerryFarmer_H
#define PokemonAutomation_PokemonSwSh_BerryFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BerryFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BerryFarmer_Descriptor();
};



class BerryFarmer : public SingleSwitchProgramInstance{
public:
    BerryFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;

    SimpleIntegerOption<uint32_t> SKIPS;
    SimpleIntegerOption<uint16_t> SAVE_ITERATIONS0;

    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif



