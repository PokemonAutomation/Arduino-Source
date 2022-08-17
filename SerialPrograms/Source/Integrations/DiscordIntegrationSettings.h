/*  Discord Integration Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordIntegrationSettings_H
#define PokemonAutomation_DiscordIntegrationSettings_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Options/BatchOption/GroupOption.h"
#include "CommonFramework/Options/BatchOption/GroupWidget.h"
#include "DiscordIntegrationTable.h"

namespace PokemonAutomation{
namespace Integration{


class DiscordIntegrationSettingsOption : public GroupOption{
public:
    DiscordIntegrationSettingsOption();
    virtual ConfigWidget* make_ui(QWidget& parent) override;

    StringOption token;
    StringOption command_prefix;
    BooleanCheckBoxOption use_suffix;
    StringOption game_status;
    StringOption hello_message;
    StringOption sudo;
    StringOption owner;
    DiscordIntegrationTable2 channels;
};
class DiscordIntegrationSettingsOptionUI : public GroupWidget{
public:
    DiscordIntegrationSettingsOptionUI(QWidget& parent, DiscordIntegrationSettingsOption& value);
    virtual void on_set_enabled(bool enabled) override;
};
inline ConfigWidget* DiscordIntegrationSettingsOption::make_ui(QWidget& parent){
    return new DiscordIntegrationSettingsOptionUI(parent, *this);
}



}
}
#endif
