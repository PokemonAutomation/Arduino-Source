/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_15_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_15_H

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

class AutoStory_Segment_15 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        BotBaseContext& context,
        AutoStoryOptions options) const override;
};



// start: At West Province Area One North Pokecenter
// end: Defeated Team Star (Dark) grunts at base entrance
void checkpoint_32(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: Defeated Team star dark grunts at base entrance
// end: Defeated Team Star (Dark) boss
void checkpoint_33(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: Defeated Team Star (Dark) boss
// end: At Cascarrafa (West) Pokecenter.
void checkpoint_34(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);  



}
}
}
#endif
