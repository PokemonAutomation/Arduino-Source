/*  AutostoryTools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Checkpoint_16_20_H
#define PokemonAutomation_PokemonSV_AutoStory_Checkpoint_16_20_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

// start: Talked to Jacq in classroom. Standing in classroom.
// end: Talked to Arven. Received Titan questline (Path of Legends). Talked to Cassiopeia. Standing in main hall.
void checkpoint_16(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Talked to Arven. Received Titan questline (Path of Legends).
// end: Talked to Cassiopeia. Saw Geeta. Talked to Nemona. Received Gym/Elite Four questline (Victory Road). Standing in staff room.
void checkpoint_17(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update); 

// start: Talked to Cassiopeia. Saw Geeta. Talked to Nemona. Received Gym/Elite Four questline (Victory Road). Standing in staff room.
// end: Talked to Clavell and the professor.
void checkpoint_18(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);          

// start: Talked to Clavell and the professor.
// end: Talked to Nemona, visited dorm, time passed.
void checkpoint_19(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);      

// start: Talked to Nemona, visited dorm, time passed.
// end: Get on ride for first time.
void checkpoint_20(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update); 

}
}
}
#endif
