/*  Alpha Gallade Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_GalladeFinder_H
#define PokemonAutomation_PokemonLA_GalladeFinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class GalladeFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GalladeFinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class GalladeFinder : public SingleSwitchProgramInstance{
public:
    GalladeFinder();
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);

private:
    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    ShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    ShinyDetectedActionOption SHINY_DETECTED_DESTINATION;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
