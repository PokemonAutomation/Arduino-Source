/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_03_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_03_H

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

class AutoStory_Segment_03 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        BotBaseContext& context,
        AutoStoryOptions options) const override;
};


// start: Battled Nemona on the beach.
// end: Met mom at gate. Received mom's sandwich.
void checkpoint_05(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Met mom at gate. Received mom's sandwich.
// end: Cleared catch tutorial.
void checkpoint_06(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);

// start: Cleared catch tutorial.
// end: Moved to cliff. Heard mystery cry.
void checkpoint_07(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);


}
}
}
#endif
