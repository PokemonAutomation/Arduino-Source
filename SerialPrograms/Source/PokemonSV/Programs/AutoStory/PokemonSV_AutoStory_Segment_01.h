/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_01_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_01_H

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

class AutoStory_Segment_01 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        BotBaseContext& context,
        AutoStoryOptions options) const override;
};

// start: stood up from chair. Walked to left side of room.
// end: standing in room. updated settings
void checkpoint_01(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update, Language language);

// start: standing in room. updated settings
// end: standing in front of power of science NPC. Cleared map tutorial.
void checkpoint_02(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: standing in front of power of science NPC. Cleared map tutorial.
// end: received starter, changed move order
void checkpoint_03(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update, Language language, StarterChoice starter_choice);


}
}
}
#endif
