/*  Ingo Battle Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_IngoBattleGrinder_H
#define PokemonAutomation_PokemonLA_IngoBattleGrinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/BatchOption/GroupOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_BattlePokemonActionTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class IngoOpponents{
    Wenton,
    Bren,
    Zisu,
    Akari_Rei,
    Kamado,
    Beni,
    Ingo,
    Ingo_Tougher,
    Mai,
    Sabi,
    Ress,
    Ingo_Alphas,
    END_LIST,
};
extern const char* INGO_OPPONENT_STRINGS[];

struct IngoOpponentMenuLocation{
    int8_t page;
    int8_t index;
};
extern const IngoOpponentMenuLocation INGO_OPPONENT_MENU_LOCATIONS_V10[];
extern const IngoOpponentMenuLocation INGO_OPPONENT_MENU_LOCATIONS_V12[];



class IngoBattleGrinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    IngoBattleGrinder_Descriptor();
};

class IngoBattleGrinder : public SingleSwitchProgramInstance{
public:
    IngoBattleGrinder(const IngoBattleGrinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    //  Returns true if version 1.0.
    bool start_dialog(ConsoleHandle& console, BotBaseContext& context);

private:
    class Stats;

    EnumDropdownOption OPPONENT;
    BattlePokemonActionTable POKEMON_ACTIONS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
