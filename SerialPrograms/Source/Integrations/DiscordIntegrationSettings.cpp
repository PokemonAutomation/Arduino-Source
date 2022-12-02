/*  Discord Integration Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "Common/Compiler.h"
#include "SleepyDiscordRunner.h"
#include "DiscordIntegrationSettings.h"

namespace PokemonAutomation{
namespace Integration{


DiscordIntegrationSettingsOption::DiscordIntegrationSettingsOption()
    : GroupOption("Discord Integration Settings", LockWhileRunning::LOCKED, true, false)
//    , m_integration_enabled(integration_enabled)
    , token(
        true,
        "<b>Discord token:</b><br>Enter your Discord bot's token. Keep it safe and don't share it with anyone.",
        LockWhileRunning::LOCKED,
        "", "0123456789abcdefghijklmnopqrstuvwxyzABCDEGFHIJKLMNOPQRSTUVWXYZ"
    )
    , command_prefix(
        false,
        "<b>Discord command prefix:</b><br>Enter a command prefix for your bot.",
        LockWhileRunning::LOCKED,
        "^", "^"
    )
    , use_suffix(
        "<b>Discord bot use suffix:</b><br>Use a suffix instead of a prefix for commands.",
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
        "<b>Discord sudo:</b><br>Enter user ID(s) to grant sudo access to.",
        LockWhileRunning::LOCKED,
        "", "123456789012345678"
    )
    , owner(
        false,
        "<b>Discord owner:</b><br>Enter the bot owner's ID (your own ID).",
        LockWhileRunning::LOCKED,
        "", "123456789012345678"
    )
{
    PA_ADD_OPTION(token);
    PA_ADD_OPTION(command_prefix);
    PA_ADD_OPTION(use_suffix);
    PA_ADD_OPTION(game_status);
    PA_ADD_OPTION(hello_message);
    PA_ADD_OPTION(sudo);
    PA_ADD_OPTION(owner);
    PA_ADD_OPTION(channels);
}
DiscordIntegrationSettingsOptionUI::DiscordIntegrationSettingsOptionUI(QWidget& parent, DiscordIntegrationSettingsOption& value)
    : GroupWidget(parent, value)
{
    QWidget* control_buttons = new QWidget(this);
    m_options_layout->insertWidget(0, control_buttons);

    QHBoxLayout* layout = new QHBoxLayout(control_buttons);

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

#ifdef PA_SLEEPY
//    this->setVisible(value.m_integration_enabled);
    set_options_enabled(value.enabled() && !SleepyDiscordRunner::is_running());

    connect(
        button_start, &QPushButton::clicked,
        this, [=, &value](bool){
            set_options_enabled(false);
            SleepyDiscordRunner::sleepy_connect();
            set_options_enabled(value.enabled() && !SleepyDiscordRunner::is_running());
        }
    );
    connect(
        button_stop, &QPushButton::clicked,
        this, [=, &value](bool){
            SleepyDiscordRunner::sleepy_terminate();
            set_options_enabled(value.enabled() && !SleepyDiscordRunner::is_running());
        }
    );
#endif
}





}
}
