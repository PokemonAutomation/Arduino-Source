/*  Day Skipper (US)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DaySkipperUS_H
#define PokemonAutomation_PokemonSwSh_DaySkipperUS_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DaySkipperUS_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    DaySkipperUS_Descriptor();
};



class DaySkipperUS : public SingleSwitchProgramInstance{
public:
    DaySkipperUS(const DaySkipperUS_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    SimpleIntegerOption<uint32_t> SKIPS;
    SimpleIntegerOption<uint16_t> REAL_LIFE_YEAR;

    EventNotificationOption NOTIFICATION_PROGRESS_UPDATE;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint16_t> CORRECTION_SKIPS;
};


}
}
}
#endif



