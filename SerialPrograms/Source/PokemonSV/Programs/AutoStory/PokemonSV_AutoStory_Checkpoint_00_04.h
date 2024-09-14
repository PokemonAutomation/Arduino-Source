/*  AutostoryTools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Checkpoint_00_04_H
#define PokemonAutomation_PokemonSV_AutoStory_Checkpoint_00_04_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


// start: after selecting character name, style and the cutscene has started
// end: stood up from chair. Walked to left side of room.
void checkpoint_00(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

// start: stood up from chair. Walked to left side of room.
// end: standing in room. updated settings
void checkpoint_01(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update, Language language);

// start: standing in room. updated settings
// end: standing in front of power of science NPC. Cleared map tutorial.
void checkpoint_02(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: standing in front of power of science NPC. Cleared map tutorial.
// end: received starter, changed move order
void checkpoint_03(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update, Language language, StarterChoice starter_choice);

// start: Received starter pokemon and changed move order. Cleared autoheal tutorial.
// end: Battled Nemona on the beach.
void checkpoint_04(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

}
}
}
#endif
