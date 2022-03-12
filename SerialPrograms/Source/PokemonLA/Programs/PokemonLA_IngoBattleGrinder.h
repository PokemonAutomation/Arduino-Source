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
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/BatchOption/GroupOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


// The program option for choosing which move style: no style, agile or strong.
class MoveStyleOption : public EnumDropdownOption{
public:
    MoveStyleOption(const char* label);
};

// A group option for all four moves of a pokemon
class PokemonBattleDecisionOption : public GroupOption{
public:
    PokemonBattleDecisionOption(QString description);

    MoveStyleOption MOVE_1;
    MoveStyleOption MOVE_2;
    MoveStyleOption MOVE_3;
    MoveStyleOption MOVE_4;

    BooleanCheckBoxOption SWITCH;
    SimpleIntegerOption<uint8_t> NUM_TURNS_TO_SWITCH;
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

    //  Returns true if version 1.0.
    bool start_dialog(SingleSwitchProgramEnvironment& env);

    // cur_pokemon: which pokemon is currently on the field, in terms of battle order
    // The first pokemon sent to the battle has cur_pokemon == 0
    // cur_move: which move to use, can be: 0, 1, 2, 3
    void use_move(const BotBaseContext &context, int cur_pokemon, int cur_move);

    void switch_pokemon(SingleSwitchProgramEnvironment& env, int& next_pokemon_in_party_order);

    const PokemonBattleDecisionOption* get_pokemon(int cur_pokemon) const;

private:
    class Stats;

    EnumDropdownOption OPPONENT;

    PokemonBattleDecisionOption POKEMON_1;
    PokemonBattleDecisionOption POKEMON_2;
    PokemonBattleDecisionOption POKEMON_3;
    PokemonBattleDecisionOption POKEMON_4;
    PokemonBattleDecisionOption POKEMON_5;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
