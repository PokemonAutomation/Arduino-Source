/*  Discord Integration Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <dpp/DPP_SilenceWarnings.h>
#include "Common/Qt/StringToolsQt.h"
//#include "CommonFramework/Globals.h"
//#include "CommonFramework/GlobalSettingsPanel.h"
#include "DppIntegration/DppClient.h"
#include "SleepyDiscordRunner.h"
#include "DiscordIntegrationSettings.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Integration{


DiscordIntegrationSettingsOption::~DiscordIntegrationSettingsOption(){
    library0.remove_listener(*this);
    this->remove_listener(*this);
}
DiscordIntegrationSettingsOption::DiscordIntegrationSettingsOption()
    : GroupOption("Discord Integration Settings", LockMode::LOCK_WHILE_RUNNING)
//    , m_integration_enabled(integration_enabled)
    , run_on_start(
        "<b>Run Discord Integration on Launch:</b><br>Automatically connect to Discord as soon as the program is launched.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , library0(
        "<b>Discord Integration Library:</b><br>Restart the program for this to take effect.",
        {
            {Library::SleepyDiscord, "sleepy", "Sleepy Discord (normal commands, deprecated)"},
            {Library::DPP, "dpp", "D++ (slash commands and normal commands)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Library::DPP
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
    , use_suffix(
        "<b>Use Suffix (Sleepy):</b><br>Use a suffix instead of a prefix for commands.",
        LockMode::LOCK_WHILE_RUNNING,
        false
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
    , sudo(
        false,
        "<b>Discord Sudo (Sleepy):</b><br>Enter user ID(s) to grant sudo access to.",
        LockMode::LOCK_WHILE_RUNNING,
        "", "123456789012345678"
    )
    , owner(
        false,
        "<b>Discord Owner (Sleepy):</b><br>Enter the bot owner's ID (your own ID).",
        LockMode::LOCK_WHILE_RUNNING,
        "", "123456789012345678"
    )
    , allow_buttons_from_users(
        "<b>Allow Buttons from Users:</b><br>Allow other users to issue button presses.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(run_on_start);
    PA_ADD_OPTION(library0);
    PA_ADD_OPTION(command_type);
    PA_ADD_OPTION(token);
    PA_ADD_OPTION(command_prefix);
    PA_ADD_OPTION(use_suffix);
    PA_ADD_OPTION(game_status);
    PA_ADD_OPTION(hello_message);
    PA_ADD_OPTION(sudo);
    PA_ADD_OPTION(owner);
    PA_ADD_OPTION(allow_buttons_from_users);
    PA_ADD_OPTION(channels);

    DiscordIntegrationSettingsOption::value_changed(this);

    this->add_listener(*this);
    library0.add_listener(*this);
}
void DiscordIntegrationSettingsOption::value_changed(void* object){
//    cout << this->enabled() << endl;
#if (defined PA_SLEEPY || defined PA_DPP)
    bool options_enabled = this->enabled();
    switch (library0){
#ifdef PA_SLEEPY
    case Library::SleepyDiscord:{
        options_enabled &= !SleepyDiscordRunner::is_running();
        ConfigOptionState state = options_enabled ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED;

        library0.set_visibility(state);
        command_type.set_visibility(ConfigOptionState::HIDDEN);
        token.set_visibility(state);
        game_status.set_visibility(state);
        hello_message.set_visibility(state);

        command_prefix.set_visibility(state);
        use_suffix.set_visibility(state);
        sudo.set_visibility(state);
        owner.set_visibility(state);
        allow_buttons_from_users.set_visibility(ConfigOptionState::HIDDEN);
        break;
    }
#endif
#ifdef PA_DPP
    case Library::DPP:{
        options_enabled &= !DppClient::Client::instance().is_initialized();
        ConfigOptionState state = options_enabled ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED;

        library0.set_visibility(state);
        command_type.set_visibility(state);
        token.set_visibility(state);
        game_status.set_visibility(state);
        hello_message.set_visibility(state);
        allow_buttons_from_users.set_visibility(state);

        command_prefix.set_visibility(state);
        use_suffix.set_visibility(ConfigOptionState::HIDDEN);
        sudo.set_visibility(ConfigOptionState::HIDDEN);
        owner.set_visibility(ConfigOptionState::HIDDEN);
        break;
    }
#endif
    default:;
    }
#endif
}


class DiscordIntegrationSettingsWidget : public GroupWidget{
public:
    DiscordIntegrationSettingsWidget(QWidget& parent, DiscordIntegrationSettingsOption& value);
};
ConfigWidget* DiscordIntegrationSettingsOption::make_QtWidget(QWidget& parent){
    return new DiscordIntegrationSettingsWidget(parent, *this);
}

DiscordIntegrationSettingsWidget::DiscordIntegrationSettingsWidget(QWidget& parent, DiscordIntegrationSettingsOption& value)
    : GroupWidget(parent, value)
{
#if (defined PA_SLEEPY || defined PA_DPP)

    QWidget* control_buttons = new QWidget(this);
    m_options_layout->insertWidget(0, control_buttons);

    QHBoxLayout* layout = new QHBoxLayout(control_buttons);
    layout->setContentsMargins(5, 5, 5, 5);

    QLabel* text = new QLabel("<b>Bot Control:</b>", control_buttons);
    layout->addWidget(text, 2);
    text->setWordWrap(true);

    QPushButton* button_start = new QPushButton("Start Bot", this);
    layout->addWidget(button_start, 1);

    QPushButton* button_stop = new QPushButton("Stop Bot", this);
    layout->addWidget(button_stop, 1);

    QFont font = button_start->font();
    font.setBold(true);
    button_start->setFont(font);
    button_stop->setFont(font);

    connect(
        button_start, &QPushButton::clicked,
        this, [this, &value](bool){
            switch (value.library0){
#ifdef PA_SLEEPY
            case DiscordIntegrationSettingsOption::Library::SleepyDiscord:
                SleepyDiscordRunner::sleepy_connect();
                break;
#endif
#ifdef PA_DPP
            case DiscordIntegrationSettingsOption::Library::DPP:
                DppClient::Client::instance().connect();
                break;
#endif
            default:;
            }
            value.value_changed(this);
        }
    );
    connect(
        button_stop, &QPushButton::clicked,
        this, [this, &value](bool){
            switch (value.library0){
#ifdef PA_SLEEPY
            case DiscordIntegrationSettingsOption::Library::SleepyDiscord:
                SleepyDiscordRunner::sleepy_terminate();
                break;
#endif
#ifdef PA_DPP
            case DiscordIntegrationSettingsOption::Library::DPP:
                DppClient::Client::instance().disconnect();
                break;
#endif
            default:;
            }
            value.value_changed(this);
        }
    );

#endif
}






std::string convert(const std::string& message){
    std::u32string ret;
    std::u32string block;
    for (char32_t ch : to_utf32(message)){
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

    return to_utf8(ret);
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
