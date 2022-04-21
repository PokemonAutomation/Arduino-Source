/*  Tree Shiny Hunting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_TreeShinyHunting_H
#define PokemonAutomation_PokemonLA_TreeShinyHunting_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class TreeShinyHunting_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    TreeShinyHunting_Descriptor();
};

class TreeShinyHunting : public SingleSwitchProgramInstance{
public:
    TreeShinyHunting(const TreeShinyHunting_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    bool check_tree(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int16_t match);
    void exit_battle(BotBaseContext& context);

private:
    class Stats;
    class RunRoute;
    ShinyDetectedActionOption MATCH_DETECTED_OPTIONS;
    EnumDropdownOption STOP_ON;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
