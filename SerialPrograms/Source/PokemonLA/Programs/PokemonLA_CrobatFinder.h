/*  Alpha Crobat Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_CrobatFinder_H
#define PokemonAutomation_PokemonLA_CrobatFinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class CrobatFinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    CrobatFinder_Descriptor();
};


class CrobatFinder : public SingleSwitchProgramInstance{
public:
    CrobatFinder(const CrobatFinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env);

private:
    class Stats;
    class RunRoute;

    ShinyDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
