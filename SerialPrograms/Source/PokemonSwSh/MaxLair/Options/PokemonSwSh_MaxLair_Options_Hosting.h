/*  Max Lair Hosting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_Hosting_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_Hosting_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/RandomCodeOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "Common/Qt/Options/GroupWidget.h"
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
    std::string check_validity(size_t consoles) const;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

    EnumDropdownOption<HostingMode> MODE;
    RandomCodeOption RAID_CODE;
    TimeExpressionOption<uint16_t> CONNECT_TO_INTERNET_DELAY;
    TimeExpressionOption<uint16_t> START_DELAY;
    TimeExpressionOption<uint16_t> LOBBY_WAIT_DELAY;

    AutoHostNotificationOption NOTIFICATIONS;
};

class HostingSettingsUI : public GroupWidget, private ConfigOption::Listener{
public:
    ~HostingSettingsUI();
    HostingSettingsUI(QWidget& parent, HostingSettings& value);
private:
    void update_option_visibility();
    virtual void value_changed() override;

    HostingSettings& m_value;
};



}
}
}
}
#endif
