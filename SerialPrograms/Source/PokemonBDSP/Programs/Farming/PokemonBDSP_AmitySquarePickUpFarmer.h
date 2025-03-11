/*  Walking Pokemon Berry Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AmitySquarePickUpFarmer_H
#define PokemonAutomation_PokemonBDSP_AmitySquarePickUpFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class AmitySquarePickUpFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AmitySquarePickUpFarmer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class AmitySquarePickUpFarmer : public SingleSwitchProgramInstance{
public:
    AmitySquarePickUpFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    SimpleIntegerOption<uint16_t> MAX_FETCH_ATTEMPTS;
    MillisecondsOption ONE_WAY_MOVING_TIME0;
    SimpleIntegerOption<uint16_t> ROUNDS_PER_FETCH;
    MillisecondsOption WAIT_TIME_FOR_POKEMON0;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
