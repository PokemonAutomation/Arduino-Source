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
#include "CommonFramework/Globals.h"
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
    library.remove_listener(*this);
}
DiscordIntegrationSettingsOption::DiscordIntegrationSettingsOption()
    : GroupOption("Discord Integration Settings", LockWhileRunning::LOCKED, true, false)
//    , m_integration_enabled(integration_enabled)
    , run_on_start(
        "<b>Run Discord integration on launch:</b><br>Automatically connect to Discord as soon as the program is launched.",
        LockWhileRunning::LOCKED,
        false
    )
    , library(
        "<b>Discord Integration Library:</b><br>Restart the program for this to take effect.",
        {
            {Library::SleepyDiscord, "sleepy", "Sleepy Discord (normal commands)"},
            {Library::DPP, "dpp", "D++ (slash commands)"},
        },
        LockWhileRunning::LOCKED,
        Library::SleepyDiscord
    )
    , token(
        true,
        "<b>Discord token:</b><br>Enter your Discord bot's token. Keep it safe and don't share it with anyone.",
        LockWhileRunning::LOCKED,
        "", "0123456789abcdefghijklmnopqrstuvwxyzABCDEGFHIJKLMNOPQRSTUVWXYZ"
    )
    , command_prefix(
        false,
        "<b>Discord command prefix (Sleepy):</b><br>Enter a command prefix for your bot.",
        LockWhileRunning::LOCKED,
        "^", "^"
    )
    , use_suffix(
        "<b>Discord bot use suffix (Sleepy):</b><br>Use a suffix instead of a prefix for commands.",
        LockWhileRunning::LOCKED,
        false
    )
    , game_status(
        false,
        "<b>Discord game status:</b><br>Enter a status message your bot would display.",
        LockWhileRunning::LOCKED,
        "", "Controlling your Switch. :)"
    )
    , hello_message(
        false,
        "<b>Discord hello message:</b><br>Enter a message you'd like the bot to respond with to the \"$hi\" command.",
        LockWhileRunning::LOCKED,
        "", "Automation at your service!"
    )
    , sudo(
        false,
        "<b>Discord sudo (Sleepy):</b><br>Enter user ID(s) to grant sudo access to.",
        LockWhileRunning::LOCKED,
        "", "123456789012345678"
    )
    , owner(
        false,
        "<b>Discord owner (Sleepy):</b><br>Enter the bot owner's ID (your own ID).",
        LockWhileRunning::LOCKED,
        "", "123456789012345678"
    )
{
    PA_ADD_OPTION(run_on_start);
    if (IS_BETA_VERSION){
        PA_ADD_OPTION(library);
    }
    PA_ADD_OPTION(token);
    PA_ADD_OPTION(command_prefix);
    PA_ADD_OPTION(use_suffix);
    PA_ADD_OPTION(game_status);
    PA_ADD_OPTION(hello_message);
    PA_ADD_OPTION(sudo);
    PA_ADD_OPTION(owner);
    PA_ADD_OPTION(channels);

    DiscordIntegrationSettingsOption::value_changed();

    library.add_listener(*this);
}
void DiscordIntegrationSettingsOption::value_changed(){
#if (defined PA_SLEEPY || defined PA_DPP)
    bool options_enabled = this->enabled();
    switch (library){
#ifdef PA_SLEEPY
    case Library::SleepyDiscord:{
        options_enabled &= !SleepyDiscordRunner::is_running();
        ConfigOptionState state = options_enabled ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED;

        library.set_visibility(state);
        token.set_visibility(state);
        game_status.set_visibility(state);
        hello_message.set_visibility(state);

        command_prefix.set_visibility(state);
        use_suffix.set_visibility(state);
        sudo.set_visibility(state);
        owner.set_visibility(state);
        break;
    }
#endif
#ifdef PA_DPP
    case Library::DPP:{
        options_enabled &= !DppClient::Client::instance().is_initialized();
        ConfigOptionState state = options_enabled ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED;

        library.set_visibility(state);
        token.set_visibility(state);
        game_status.set_visibility(state);
        hello_message.set_visibility(state);

        command_prefix.set_visibility(ConfigOptionState::HIDDEN);
        use_suffix.set_visibility(ConfigOptionState::HIDDEN);
        sudo.set_visibility(ConfigOptionState::HIDDEN);
        owner.set_visibility(ConfigOptionState::HIDDEN);
        break;
    }
#endif
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
        this, [&value](bool) {
            switch (value.library){
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
            }
            value.value_changed();
        }
    );
    connect(
        button_stop, &QPushButton::clicked,
        this, [&value](bool) {
            switch (value.library){
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
            }
            value.value_changed();
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
std::string MessageBuilder::build_message(bool ping, const std::string& user_id, const std::string& message) const{
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

    return str;
}






}
}
