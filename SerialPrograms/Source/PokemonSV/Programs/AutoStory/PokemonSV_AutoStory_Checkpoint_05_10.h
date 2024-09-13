/*  AutostoryTools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Checkpoint_05_10_H
#define PokemonAutomation_PokemonSV_AutoStory_Checkpoint_05_10_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


// start: Battled Nemona on the beach.
// end: Met mom at gate. Received mom's sandwich.
void checkpoint_05(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Met mom at gate. Received mom's sandwich.
// end: Cleared catch tutorial.
void checkpoint_06(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Cleared catch tutorial.
// end: Moved to cliff. Heard mystery cry.
void checkpoint_07(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Moved to cliff. Heard mystery cry.
// end: Rescued Koraidon/Miraidon and escaped from the Houndoom Cave.
void checkpoint_08(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Rescued Koraidon/Miraidon and escaped from the Houndoom Cave.
// end: Battled Arven and received Legendary's Pokeball.
void checkpoint_09(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Battled Arven and received Legendary's Pokeball.
// end: Talked to Nemona at the Lighthouse.
void checkpoint_10(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

}
}
}
#endif
