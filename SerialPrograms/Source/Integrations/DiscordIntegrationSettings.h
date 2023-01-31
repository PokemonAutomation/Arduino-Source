/*  Discord Integration Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DiscordIntegrationSettings_H
#define PokemonAutomation_DiscordIntegrationSettings_H

#include <set>
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



class MessageBuilder{
public:
    MessageBuilder(const std::vector<std::string>& message_tags);

    bool should_send(const std::vector<std::string>& channel_tags) const;
    std::string build_message(bool ping, const std::string& user_id, const std::string& message) const;

private:
    std::set<std::string> m_message_tags;
};





}
}
#endif
