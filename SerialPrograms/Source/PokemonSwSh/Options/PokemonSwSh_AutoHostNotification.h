/*  Auto-Host Notification
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHostNotification_H
#define PokemonAutomation_PokemonSwSh_AutoHostNotification_H

#include "CommonFramework/Options/BatchOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/StringBoxOption.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "NintendoSwitch/Options/FriendCodeListOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class AutoHostNotificationOption : public GroupOption{
public:
    AutoHostNotificationOption(QString label);

    BooleanCheckBoxOption ENABLE;
    StringBoxOption DESCRIPTION;
    BooleanCheckBoxOption SHOW_STATS;
    ScreenshotOption SCREENSHOT;
};




}
}
}
#endif
