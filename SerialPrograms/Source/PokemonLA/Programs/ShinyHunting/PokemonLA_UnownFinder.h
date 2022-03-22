/*  Unown Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_UnownFinder_H
#define PokemonAutomation_PokemonLA_UnownFinder_H
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/PokemonLA_UnderAttackDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class UnownFinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    UnownFinder_Descriptor();
};

class UnownFinder : public SingleSwitchProgramInstance{
public:
    UnownFinder(const UnownFinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env);

private:
    class Stats;
    class RunRoute;
    ShinyDetectedActionOption SHINY_DETECTED;
    BooleanCheckBoxOption SKIP_PATH_SHINY;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
