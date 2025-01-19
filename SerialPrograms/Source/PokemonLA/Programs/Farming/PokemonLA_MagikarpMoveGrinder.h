/*  Magikarp Move Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Battle a magikarp to grind move related research tasks.
 */

#ifndef PokemonAutomation_PokemonLA_MagikarpMoveGrinder_H
#define PokemonAutomation_PokemonLA_MagikarpMoveGrinder_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_BattlePokemonActionTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class MagikarpMoveGrinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MagikarpMoveGrinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class MagikarpMoveGrinder : public SingleSwitchProgramInstance{
public:
    MagikarpMoveGrinder();
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    void battle_magikarp(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);

    void grind_mimic(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);

private:
    OneMoveBattlePokemonActionTable POKEMON_ACTIONS;

    BooleanCheckBoxOption SPECIAL_CASE_MIMIC;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
