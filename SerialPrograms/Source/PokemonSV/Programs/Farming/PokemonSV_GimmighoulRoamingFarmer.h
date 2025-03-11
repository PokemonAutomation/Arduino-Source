/*  Gimmighoul Roaming Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_GimmighoulRoamingFarm_H
#define PokemonAutomation_PokemonSV_GimmighoulRoamingFarm_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class GimmighoulRoamingFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GimmighoulRoamingFarmer_Descriptor();
};

class GimmighoulRoamingFarmer : public SingleSwitchProgramInstance{
public:
    GimmighoulRoamingFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint32_t> SKIPS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    BooleanCheckBoxOption FIX_TIME_WHEN_DONE;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif



