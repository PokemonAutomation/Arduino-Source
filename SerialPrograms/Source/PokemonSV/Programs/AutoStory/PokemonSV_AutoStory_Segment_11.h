/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_11_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_11_H

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

class AutoStory_Segment_11 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        BotBaseContext& context,
        AutoStoryOptions options) const override;
};


// start: At Cortondo East Pokecenter
// end: Spoke to Cortondo Gym reception. At Cortondo West Pokecenter.
void checkpoint_24(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: Spoke to Cortondo Gym reception. At Cortondo West Pokecenter.
// end: Defeated the trainers at Olive Roll, but left Olive unmoved. Then backed out, standing in front of the Olive Roll NPC.
void checkpoint_25(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  

// start: Defeated the trainers at Olive Roll, but left Olive unmoved. Then backed out, standing in front of the Olive Roll NPC.
// end: Completed Olive roll gym challenge.
void checkpoint_26(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  

// start: Completed Olive roll gym challenge. 
// end: At Cortondo East Pokecenter.
void checkpoint_27(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  



}
}
}
#endif
