/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_16_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_16_H

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

class AutoStory_Segment_16 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        BotBaseContext& context,
        AutoStoryOptions options) const override;
};


// start: At Cascarrafa (West) Pokecenter.
// end: At Cascarrafa Gym. Received Kofu's wallet.
void checkpoint_35(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);   

// start: At Cascarrafa Gym. Received Kofu's wallet.
// end: At Porto Marinada Pokecenter.
void checkpoint_36(SingleSwitchProgramEnvironment& env, BotBaseContext& context, EventNotificationOption& notif_status_update);             





}
}
}
#endif
