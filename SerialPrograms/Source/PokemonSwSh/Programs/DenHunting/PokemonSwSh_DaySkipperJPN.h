/*  Day Skipper (JPN)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DaySkipperJPN_H
#define PokemonAutomation_PokemonSwSh_DaySkipperJPN_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DaySkipperJPN_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    DaySkipperJPN_Descriptor();
};



class DaySkipperJPN : public SingleSwitchProgramInstance{
public:
    DaySkipperJPN(const DaySkipperJPN_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, const BotBaseContext& context) override;

private:
    SimpleIntegerOption<uint32_t> SKIPS;

    EventNotificationOption NOTIFICATION_PROGRESS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint16_t> CORRECTION_SKIPS;
};


}
}
}
#endif



