/*  Walking Pokemon Berry Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AmitySquarePickUpFarmer_H
#define PokemonAutomation_PokemonBDSP_AmitySquarePickUpFarmer_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class AmitySquarePickUpFarmer_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    AmitySquarePickUpFarmer_Descriptor();
};


class AmitySquarePickUpFarmer : public SingleSwitchProgramInstance{
public:
    AmitySquarePickUpFarmer(const AmitySquarePickUpFarmer_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;


private:
    struct Stats;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    SimpleIntegerOption<uint16_t> MAX_FETCH_ATTEMPTS;
    TimeExpressionOption<uint16_t> ONE_WAY_MOVING_TIME;
    SimpleIntegerOption<uint16_t> ROUNDS_PER_FETCH;
    TimeExpressionOption<uint16_t> WAIT_TIME_FOR_POKEMON;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
