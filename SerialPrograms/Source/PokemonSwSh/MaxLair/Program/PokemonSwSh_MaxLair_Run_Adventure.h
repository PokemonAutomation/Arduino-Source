/*  Max Lair Run Adventure
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_Adventure_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_Adventure_H

#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


enum class AdventureResult{
    FINISHED,
    STOP_PROGRAM,
    START_ERROR,
};


void loop_adventures(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    const Consoles& consoles,
    size_t host_index, size_t boss_slot,
    const EndBattleDecider& decider,
    bool go_home_when_done,
    HostingSettings& HOSTING,
    TouchDateIntervalOption& TOUCH_DATE_INTERVAL,
    EventNotificationOption& notification_status,
    EventNotificationOption& notification_shiny
);


}
}
}
}
#endif
