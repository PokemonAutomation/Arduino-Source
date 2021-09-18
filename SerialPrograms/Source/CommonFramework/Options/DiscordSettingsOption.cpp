/*  Discord Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QLabel>
#include <QPushButton>
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/ProgramNotifications.h"
#include "DiscordSettingsOption.h"

namespace PokemonAutomation{


DiscordSettingsOption::DiscordSettingsOption()
    : GroupOption("Discord Notification Settings")
    , instance_name(
        "<b>Instance Name:</b><br>If you are running multiple instances of this program, give it a name to distinguish them in notifications.",
        ""
    )
    , ping_once(
        "<b>Ping on 1st URL Only:</b><br>Ping on the 1st URL in the list below only. Otherwise, ping on all URLs.",
        true
    )
    , webhook_urls(
        "<b>Discord Webhook URLs:</b><br>Enter the webhook URLs here. One URL per line. Notifications are sent to all them.",
        ""
    )
    , user_id(
        "<b>Discord User ID:</b><br>Set this to your discord user ID to receive pings. Your ID is a number.",
        ""
    )
    , message(
        "<b>Discord Message:</b><br>Message to put on every discord notification.",
        ""
    )
{
    PA_ADD_OPTION(instance_name);
    PA_ADD_OPTION(ping_once);
    PA_ADD_OPTION(webhook_urls);
    PA_ADD_OPTION(user_id);
    PA_ADD_OPTION(message);
}


DiscordSettingsOptionUI::DiscordSettingsOptionUI(QWidget& parent, DiscordSettingsOption& value)
    : GroupOptionUI(parent, value)
{
    QWidget* widget = new QWidget(this);
    m_options_layout->addWidget(widget);

    QHBoxLayout* layout = new QHBoxLayout(widget);

    QLabel* text = new QLabel("<b>Test Messages:</b>", this);
    layout->addWidget(text, 2);
    text->setWordWrap(true);

//    layout->addStretch(1);

    QPushButton* button0 = new QPushButton("Send non-pinging notification.", this);
    layout->addWidget(button0, 1);

//    layout->addStretch(1);

    QPushButton* button1 = new QPushButton("Send pinging notification.", this);
    layout->addWidget(button1, 1);

//    layout->addStretch(1);

    connect(
        button0, &QPushButton::clicked,
        this, [=](bool){
            QJsonObject embeds;
            send_program_notification(
                global_logger(), false, Qt::green,
                "Webhook Test", "Webhook Test",
                {
                    {"Message", "Hello World!"},
                }
            );
        }
    );
    connect(
        button1, &QPushButton::clicked,
        this, [=](bool){
            QJsonObject embeds;
            send_program_notification(
                global_logger(), true, Qt::green,
                "Webhook Test", "Webhook Test",
                {
                    {"Message", "Hello World!\nDid this message ping you? If not, your User ID is set incorrectly."},
                }
            );
        }
    );
}




}

