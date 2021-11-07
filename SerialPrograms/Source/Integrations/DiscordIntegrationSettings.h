/*  Discord Integration Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordIntegrationSettings_H
#define PokemonAutomation_DiscordIntegrationSettings_H

#include "CommonFramework/Options/BatchOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/StringOption.h"
#include "DiscordIntegrationTable.h"

namespace PokemonAutomation{
namespace Integration{


class DiscordIntegrationSettingsOption : public GroupOption{
public:
    DiscordIntegrationSettingsOption(bool integration_enabled);
    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

    bool m_integration_enabled = false;
    StringOption token;
    StringOption command_prefix;
    BooleanCheckBoxOption use_suffix;
    StringOption game_status;
    StringOption hello_message;
    StringOption sudo;
    StringOption owner;
    DiscordIntegrationTable channels;
};
class DiscordIntegrationSettingsOptionUI : public GroupOptionUI{
public:
    DiscordIntegrationSettingsOptionUI(QWidget& parent, DiscordIntegrationSettingsOption& value);
    virtual void on_set_enabled(bool enabled) override;
};
inline ConfigOptionUI* DiscordIntegrationSettingsOption::make_ui(QWidget& parent){
    return new DiscordIntegrationSettingsOptionUI(parent, *this);
}



}
}
#endif
