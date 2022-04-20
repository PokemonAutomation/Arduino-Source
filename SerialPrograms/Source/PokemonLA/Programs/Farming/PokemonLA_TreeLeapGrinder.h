/*  Tree Leap Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_TreeLeapGrinder_H
#define PokemonAutomation_PokemonLA_TreeLeapGrinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/PokemonLA_UnderAttackDetector.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class TreeLeapGrinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    TreeLeapGrinder_Descriptor();
};

class TreeLeapGrinder : public SingleSwitchProgramInstance{
public:
    TreeLeapGrinder(const TreeLeapGrinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    bool check_tree(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int16_t match);
    void exit_battle(BotBaseContext& context);

private:
    class Stats;
    class RunRoute;

    ShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    ShinyDetectedActionOption MATCH_DETECTED_OPTIONS;
    SimpleIntegerOption<uint8_t> SHAKES;
    EnumDropdownOption STOP_ON;
    EnumDropdownOption POKEMON;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
/*  Tree Leap Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_TreeLeapGrinder_H
#define PokemonAutomation_PokemonLA_TreeLeapGrinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/PokemonLA_UnderAttackDetector.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class TreeLeapGrinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    TreeLeapGrinder_Descriptor();
};

class TreeLeapGrinder : public SingleSwitchProgramInstance{
public:
    TreeLeapGrinder(const TreeLeapGrinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    bool check_tree(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int16_t match);
    void exit_battle(BotBaseContext& context);

private:
    class Stats;
    class RunRoute;

    ShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    ShinyDetectedActionOption MATCH_DETECTED_OPTIONS;
    SimpleIntegerOption<uint8_t> SHAKES;
    EnumDropdownOption STOP_ON;
    EnumDropdownOption POKEMON;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
