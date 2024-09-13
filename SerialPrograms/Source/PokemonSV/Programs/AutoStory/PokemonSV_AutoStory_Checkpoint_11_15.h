/*  AutostoryTools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Checkpoint_11_15_H
#define PokemonAutomation_PokemonSV_AutoStory_Checkpoint_11_15_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


// start: Talked to Nemona at the Lighthouse.
// end: Arrived at Los Platos pokecenter. Cleared Let's go tutorial.
void checkpoint_11(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Arrived at Los Platos pokecenter. Cleared Let's go tutorial.
// end: Arrived at Mesagoza (South) Pokecenter
void checkpoint_12(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Arrived at Mesagoza (South) Pokecenter
// end: Battled Nemona at Mesagoza gate. Entered Mesagoza.
void checkpoint_13(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Battled Nemona at Mesagoza gate. Entered Mesagoza.
// end: Battled Team Star at school entrance.
void checkpoint_14(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Battled Team Star at school entrance.
// end: Talked to Jacq in classroom. Standing in classroom.
void checkpoint_15(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

}
}
}
#endif
