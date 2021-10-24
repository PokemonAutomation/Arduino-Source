/*  Max Lair Run Adventure
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_Adventure_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_Adventure_H

#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


enum class AdventureResult{
    FINISHED,
    STOP_PROGRAM,
    ERROR,
};

AdventureResult run_adventure(
    MultiSwitchProgramEnvironment& env,
    const QString& program_name,
    size_t host_index, size_t boss_slot,
    const MaxLairConsoleOptions& player0,
    const MaxLairConsoleOptions& player1,
    const MaxLairConsoleOptions& player2,
    const MaxLairConsoleOptions& player3,
    HostingSettings& HOSTING,
    const EndBattleDecider& decider,
    EventNotificationOption& notification_noshiny,
    EventNotificationOption& notification_shiny,
    bool save_path_if_host,
    bool return_to_entrance, bool go_home_when_done
);


}
}
}
}
#endif
