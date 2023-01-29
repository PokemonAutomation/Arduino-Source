/*  Discord Integration Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordIntegrationSettings_H
#define PokemonAutomation_DiscordIntegrationSettings_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Qt/Options/GroupWidget.h"
#include "DiscordIntegrationTable.h"

namespace PokemonAutomation{
namespace Integration{


class DiscordIntegrationSettingsOption : public GroupOption, private ConfigOption::Listener{
public:
    ~DiscordIntegrationSettingsOption();
    DiscordIntegrationSettingsOption();
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;
    virtual void value_changed() override;

    BooleanCheckBoxOption run_on_start;

    enum class Library{
        SleepyDiscord,
        DPP,
    };
    EnumDropdownOption<Library> library;

    StringOption token;
    StringOption command_prefix;
    BooleanCheckBoxOption use_suffix;
    StringOption game_status;
    StringOption hello_message;
    StringOption sudo;
    StringOption owner;
    DiscordIntegrationTable channels;
};
class DiscordIntegrationSettingsOptionUI : public GroupWidget{
public:
    DiscordIntegrationSettingsOptionUI(QWidget& parent, DiscordIntegrationSettingsOption& value);
};
inline ConfigWidget* DiscordIntegrationSettingsOption::make_QtWidget(QWidget& parent){
    return new DiscordIntegrationSettingsOptionUI(parent, *this);
}



}
}
#endif
