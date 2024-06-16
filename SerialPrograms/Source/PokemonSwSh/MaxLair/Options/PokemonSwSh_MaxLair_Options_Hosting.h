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

class HostingSettings : public GroupOption, private ConfigOption::Listener{
public:
    ~HostingSettings();
    HostingSettings();
    using GroupOption::check_validity;
    std::string check_validity(size_t consoles) const;

    virtual void value_changed(void* object) override;

    EnumDropdownOption<HostingMode> MODE;
    RandomCodeOption RAID_CODE;
    TimeExpressionOption<uint16_t> CONNECT_TO_INTERNET_DELAY;
    TimeExpressionOption<uint16_t> START_DELAY;
    TimeExpressionOption<uint16_t> LOBBY_WAIT_DELAY;

    AutoHostNotificationOption NOTIFICATIONS;
};



}
}
}
}
#endif
