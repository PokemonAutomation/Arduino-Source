/*  Wigglytuff Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_WigglytuffFarmer_H
#define PokemonAutomation_PokemonLZA_WigglytuffFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonLZA/Options/PokemonLZA_BattleAIOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class WigglytuffFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WigglytuffFarmer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class WigglytuffFarmer : public SingleSwitchProgramInstance{
public:
    WigglytuffFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // Handle all the logic of talking to Bain the rich boy.
    // Return true when the user clicks the button STOP_AFTER_CURRENT to stop or the required amount of rounds is
    // reached. Return false when it enters battle.
    bool run_lobby(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    // Handle all battle logic.
    void run_round(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    DeferredStopButtonOption STOP_AFTER_CURRENT;
    SimpleIntegerOption<uint32_t> NUM_ROUNDS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    SimpleIntegerOption<uint32_t> PERIODIC_SAVE;

    BattleAIOption BATTLE_AI;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
