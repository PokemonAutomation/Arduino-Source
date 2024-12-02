/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_20_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_20_H

#include <functional>
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_20 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        BotBaseContext& context,
        AutoStoryOptions options) const override;
};


// start: Defeated Klawf. At Artazon (West) Pokecenter.
// end: At Artazon Gym building. Battled Nemona. Received Sunflora gym challenge.
void checkpoint_43(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  

// start: At Artazon Gym building. Battled Nemona. Received Sunflora gym challenge.
// end: Finished Sunflora gym challenge.
void checkpoint_44(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  

// start: Finished Sunflora gym challenge.
// end: Defeated Artazon Gym (Grass). Inside gym building.
void checkpoint_45(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update); 

// start: Defeated Artazon Gym (Grass). Inside gym building.
// end: 
void checkpoint_46(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  

// start: 
// end: 
void checkpoint_47(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  

// start: 
// end: 
void checkpoint_48(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  

// start: 
// end: 
void checkpoint_49(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  



}
}
}
#endif
