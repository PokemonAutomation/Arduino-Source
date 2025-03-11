/*  BBQ Solo Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BBQSoloFarmer_H
#define PokemonAutomation_PokemonSV_BBQSoloFarmer_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_BBQOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class BBQSoloFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BBQSoloFarmer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class BBQSoloFarmer : public SingleSwitchProgramInstance{
public:
    BBQSoloFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    BBQOption BBQ_OPTIONS;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif
