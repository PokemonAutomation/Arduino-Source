/*  Magikarp Move Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Battle a magikarp to grind move related research tasks.
 */

#ifndef PokemonAutomation_PokemonLA_MagikarpMoveGrinder_H
#define PokemonAutomation_PokemonLA_MagikarpMoveGrinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class MagikarpMoveGrinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    MagikarpMoveGrinder_Descriptor();
};

class MagikarpMoveGrinder : public SingleSwitchProgramInstance{
public:
    MagikarpMoveGrinder(const MagikarpMoveGrinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void battle_magikarp(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void grind_mimic(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    class Stats;

    OneMoveBattlePokemonActionTable POKEMON_ACTIONS;

    BooleanCheckBoxOption SPECIAL_CASE_MIMIC;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
