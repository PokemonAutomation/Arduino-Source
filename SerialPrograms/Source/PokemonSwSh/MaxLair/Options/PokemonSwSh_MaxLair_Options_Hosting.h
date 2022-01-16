/*  Max Lair Hosting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_Hosting_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_Hosting_H

#include "CommonFramework/Options/BatchOption/GroupOption.h"
#include "CommonFramework/Options/BatchOption/GroupWidget.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/RandomCodeOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



enum class HostingMode{
    NOT_HOSTING,
    HOST_LOCALLY,
    HOST_ONLINE,
};

class HostingSettings : public GroupOption{
public:
    HostingSettings();
    using GroupOption::check_validity;
    QString check_validity(size_t consoles) const;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

    EnumDropdownOption MODE;
    RandomCodeOption RAID_CODE;
    TimeExpressionOption<uint16_t> CONNECT_TO_INTERNET_DELAY;
    TimeExpressionOption<uint16_t> START_DELAY;
    TimeExpressionOption<uint16_t> LOBBY_WAIT_DELAY;

    AutoHostNotificationOption NOTIFICATIONS;
};

class HostingSettingsUI : public GroupWidget{
public:
    HostingSettingsUI(QWidget& parent, HostingSettings& value);
private:
    void update_option_visibility();
};



}
}
}
}
#endif
