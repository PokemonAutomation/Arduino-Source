/*  Auto-Host Notification
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHostNotification_H
#define PokemonAutomation_PokemonSwSh_AutoHostNotification_H

#include "CommonFramework/Options/BatchOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/TextEditOption.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_FriendCodeListOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class AutoHostNotificationOption : public GroupOption{
public:
    AutoHostNotificationOption(QString label, bool max_lair);

    TextEditOption DESCRIPTION;
    BooleanCheckBoxOption SHOW_STATS;
//    ScreenshotOption SCREENSHOT;

    EventNotificationOption NOTIFICATION;
};




}
}
}
#endif
