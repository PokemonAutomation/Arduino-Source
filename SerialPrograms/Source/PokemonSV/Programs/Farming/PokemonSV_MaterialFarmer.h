/*  Material Farmer - Happiny dust
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MaterialFarmer_H
#define PokemonAutomation_PokemonSV_MaterialFarmer_H

#include <functional>
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"
#include "PokemonSV_MaterialFarmerTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class MaterialFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MaterialFarmer_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class MaterialFarmer : public SingleSwitchProgramInstance{
public:
    MaterialFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    MaterialFarmerOptions MATERIAL_FARMER_OPTIONS;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
