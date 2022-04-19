/*  Alpha Gallade Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_GalladeFinder_H
#define PokemonAutomation_PokemonLA_GalladeFinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class GalladeFinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    GalladeFinder_Descriptor();
};


class GalladeFinder : public SingleSwitchProgramInstance{
public:
    GalladeFinder(const GalladeFinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    class Stats;

    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    ShinyDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
