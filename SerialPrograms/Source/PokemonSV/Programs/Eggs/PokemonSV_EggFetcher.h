/*  Egg Fetcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_EggFetcher_H
#define PokemonAutomation_PokemonSV_EggFetcher_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class EggFetcher_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggFetcher_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class EggFetcher : public SingleSwitchProgramInstance{
public:
    EggFetcher();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    size_t fetch_eggs(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    SimpleIntegerOption<uint16_t> EGGS_TO_FETCH;
    TimeExpressionOption<uint32_t> FETCH_PERIOD;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
