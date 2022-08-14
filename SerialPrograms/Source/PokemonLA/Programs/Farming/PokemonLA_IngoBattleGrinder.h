/*  Ingo Battle Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_IngoBattleGrinder_H
#define PokemonAutomation_PokemonLA_IngoBattleGrinder_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/BatchOption/GroupOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
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



class IngoBattleGrinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    IngoBattleGrinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class IngoBattleGrinder : public SingleSwitchProgramInstance{
public:
    IngoBattleGrinder();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, std::map<size_t, size_t>& pokemon_move_attempts);

    //  Returns true if version 1.0.
    bool start_dialog(ConsoleHandle& console, BotBaseContext& context);

private:
    EnumDropdownOption OPPONENT;
    BattlePokemonActionTable POKEMON_ACTIONS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
