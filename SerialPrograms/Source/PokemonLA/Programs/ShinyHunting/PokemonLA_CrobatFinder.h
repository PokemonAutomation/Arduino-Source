/*  Alpha Crobat Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_CrobatFinder_H
#define PokemonAutomation_PokemonLA_CrobatFinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class CrobatFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    CrobatFinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class CrobatFinder : public SingleSwitchProgramInstance{
public:
    CrobatFinder();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    OverworldShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    OverworldShinyDetectedActionOption SHINY_DETECTED_DESTINATION;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
