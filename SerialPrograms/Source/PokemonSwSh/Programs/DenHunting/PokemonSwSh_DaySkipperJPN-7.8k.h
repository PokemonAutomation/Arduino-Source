/*  Day Skipper (JPN) - 7.8k version
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DaySkipperJPN7p8k_H
#define PokemonAutomation_PokemonSwSh_DaySkipperJPN7p8k_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/DateOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DaySkipperJPN7p8k_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DaySkipperJPN7p8k_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class DaySkipperJPN7p8k : public SingleSwitchProgramInstance{
public:
    DaySkipperJPN7p8k();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint32_t> SKIPS;
    DateTimeOption START_DATE;

    EventNotificationOption NOTIFICATION_PROGRESS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint16_t> CORRECTION_SKIPS;
};


}
}
}
#endif



