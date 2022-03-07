/*  Money Farmer (Highlands)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_IngoBattleGrinder_H
#define PokemonAutomation_PokemonLA_IngoBattleGrinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "CommonFramework/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class MoveStyleOption : public EnumDropdownOption{
public:
    MoveStyleOption(const char* label);
};

class IngoBattleGrinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    IngoBattleGrinder_Descriptor();
};


class IngoBattleGrinder : public SingleSwitchProgramInstance{
public:
    IngoBattleGrinder(const IngoBattleGrinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env);

    void use_move(const BotBaseContext &context, int cur_move, bool intended_pokemon_fainted);

private:
    class Stats;

    MoveStyleOption MOVE1_STYLE_OPTION;
    MoveStyleOption MOVE2_STYLE_OPTION;
    MoveStyleOption MOVE3_STYLE_OPTION;
    MoveStyleOption MOVE4_STYLE_OPTION;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
