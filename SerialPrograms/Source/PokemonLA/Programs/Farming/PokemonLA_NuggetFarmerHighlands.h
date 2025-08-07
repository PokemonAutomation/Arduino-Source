/*  Money Farmer (Highlands)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_MoneyFarmerHighlands_H
#define PokemonAutomation_PokemonLA_MoneyFarmerHighlands_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class NuggetFarmerHighlands_Descriptor : public SingleSwitchProgramDescriptor{
public:
    NuggetFarmerHighlands_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class NuggetFarmerHighlands : public SingleSwitchProgramInstance{
public:
    NuggetFarmerHighlands();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    bool run_iteration(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        bool fresh_from_reset
    );

private:
    class RunRoute;

    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    OverworldShinyDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
