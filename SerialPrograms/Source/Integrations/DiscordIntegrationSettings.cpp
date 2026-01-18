/*  Discord Integration Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Strings/Unicode.h"
#include "Common/Qt/StringToolsQt.h"
#include "Integrations/DppIntegration/DppClient.h"
#include "DiscordIntegrationSettings.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Integration{


DiscordIntegrationSettingsOption::~DiscordIntegrationSettingsOption(){
    this->remove_listener(*this);
}
DiscordIntegrationSettingsOption::DiscordIntegrationSettingsOption()
    : ConfigOptionImpl<DiscordIntegrationSettingsOption, GroupOption>(
        "Discord Integration Settings",
        LockMode::LOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_DISABLED
    )
//    , m_integration_enabled(integration_enabled)
    , run_on_start(
        "<b>Run Discord Integration on Launch:</b><br>Automatically connect to Discord as soon as the program is launched.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , command_type(
        "<b>Discord Integration Command Type:</b><br>Restart the program for this to take effect.",
        {
            {CommandType::SlashCommands, "slash", "Slash commands"},
            {CommandType::MessageCommands, "message", "Message (prefix) commands"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        CommandType::SlashCommands
    )
    , token(
        true,
        "<b>Discord Token:</b><br>Enter your Discord bot's token. Keep it safe and don't share it with anyone.",
        LockMode::LOCK_WHILE_RUNNING,
        "", "0123456789abcdefghijklmnopqrstuvwxyzABCDEGFHIJKLMNOPQRSTUVWXYZ"
    )
    , command_prefix(
        false,
        "<b>Discord Command Prefix:</b><br>Enter a command prefix for your bot.",
        LockMode::LOCK_WHILE_RUNNING,
        "^", "^"
    )
    , game_status(
        false,
        "<b>Discord Game Status:</b><br>Enter a status message your bot would display.",
        LockMode::LOCK_WHILE_RUNNING,
        "", "Controlling your Switch. :)"
    )
    , hello_message(
        false,
        "<b>Discord Hello Message:</b><br>Enter a message you'd like the bot to respond with to the \"$hi\" command.",
        LockMode::LOCK_WHILE_RUNNING,
        "", "Automation at your service!"
    )
    , allow_buttons_from_users(
        "<b>Allow Buttons from Users:</b><br>Allow other users to issue button presses.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(run_on_start);
    PA_ADD_OPTION(command_type);
    PA_ADD_OPTION(token);
    PA_ADD_OPTION(command_prefix);
    PA_ADD_OPTION(game_status);
    PA_ADD_OPTION(hello_message);
    PA_ADD_OPTION(allow_buttons_from_users);
    PA_ADD_OPTION(channels);

    DiscordIntegrationSettingsOption::on_config_value_changed(this);

    this->add_listener(*this);
}
void DiscordIntegrationSettingsOption::on_config_value_changed([[maybe_unused]] void* object){
//    cout << this->enabled() << endl;
#ifdef PA_DPP
    bool options_enabled = this->enabled();
    options_enabled &= !DppClient::Client::instance().is_initialized();
    ConfigOptionState state = options_enabled ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED;

    command_type.set_visibility(state);
    token.set_visibility(state);
    game_status.set_visibility(state);
    hello_message.set_visibility(state);
    allow_buttons_from_users.set_visibility(state);

    command_prefix.set_visibility(state);
#endif
}







std::string convert(const std::string& message){
    std::u32string ret;
    std::u32string block;
    for (char32_t ch : utf8_to_utf32(message)){
        switch (ch){
        case 38:
            if (block.empty() || block.back() != 64){
                break;
            }
            continue;
        case 64:
            block += ch;
            continue;
        }
        ret += ch;
        block.clear();
    }

    return utf32_to_str(ret);
}


MessageBuilder::MessageBuilder(const std::vector<std::string>& message_tags){
    for (const std::string& tag : message_tags){
        m_message_tags.insert(to_lower(tag));
    }
}
bool MessageBuilder::should_send(const std::vector<std::string>& channel_tags) const{
    for (const std::string& tag : channel_tags){
        auto iter = m_message_tags.find(to_lower(tag));
        if (iter != m_message_tags.end()){
            return true;
        }
    }
    return false;
}
std::string MessageBuilder::build_message(
    std::chrono::seconds delay,
    bool ping, const std::string& user_id,
    const std::string& message
) const{
    if (std::atoll(user_id.c_str()) == 0){
        ping = false;
    }

    std::string str;
    if (ping){
        str += "<?" + user_id + ">";
        str[1]++;
    }

    if (!message.empty()){
        if (!str.empty()){
            str += " ";
        }
        str += convert(message);
    }

    auto delay_seconds = delay.count();
    if (delay_seconds != 0){
        if (!str.empty()){
            str += "\n";
        }
        str += "*(";
        str += "Message ";
        str += "delay";
        str += "ed ";
        str += "by " + std::to_string(delay_seconds);
        str += delay_seconds == 1
            ? " second"
            : " seconds";
        str += ".)*";
    }

    return str;
}






}
}
