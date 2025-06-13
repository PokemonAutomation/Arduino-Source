/*  Day Skipper (US)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DaySkipperUS_H
#define PokemonAutomation_PokemonSwSh_DaySkipperUS_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DaySkipperUS_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DaySkipperUS_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class DaySkipperUS : public SingleSwitchProgramInstance{
public:
    DaySkipperUS();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void run_switch1(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void run_switch2(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    SimpleIntegerOption<uint32_t> SKIPS;
    SimpleIntegerOption<uint16_t> REAL_LIFE_YEAR;

    EventNotificationOption NOTIFICATION_PROGRESS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint16_t> CORRECTION_SKIPS;
};


}
}
}
#endif



