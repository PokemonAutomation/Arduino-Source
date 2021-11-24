/*  Discord Webhook Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCheckBox>
#include <QLineEdit>
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "DiscordWebhookSettings.h"

namespace PokemonAutomation{
namespace Integration{



DiscordWebhookUrl::DiscordWebhookUrl()
    : enabled(true)
    , ping(true)
    , tags({"Notifs", "Showcase", "LiveHost"})
{}

void DiscordWebhookUrl::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    json_get_bool(enabled, obj, "Enabled");
    json_get_string(label, obj, "Label");
    json_get_bool(ping, obj, "Ping");
    QJsonArray array = json_get_array_nothrow(obj, "Tags");
    tags.clear();
    for (const auto& tag : array){
        QString token = EventNotificationSettings::sanitize_tag(tag.toString());
        if (!token.isEmpty()){
            tags.emplace_back(std::move(token));
        }
    }
#if 0
    if (tags.empty()){
        tags.emplace_back("Notifs");
        tags.emplace_back("Showcase");
    }
#endif
    json_get_string(url, obj, "URL");
}
QJsonValue DiscordWebhookUrl::to_json() const{
    QJsonObject obj;
    obj["Enabled"] = enabled;
    obj["Label"] = label;
    obj["Ping"] = ping;
    QJsonArray array;
    for (const QString& tag : tags){
        array.append(tag);
    }
    obj.insert("Tags", array);
    obj["URL"] = url;
    return obj;
}
std::unique_ptr<EditableTableRow> DiscordWebhookUrl::clone() const{
    return std::unique_ptr<EditableTableRow>(new DiscordWebhookUrl(*this));
}
std::vector<QWidget*> DiscordWebhookUrl::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets;
    widgets.emplace_back(make_enabled_box(parent));
    widgets.emplace_back(make_label_box(parent));
    widgets.emplace_back(make_ping_box(parent));
    widgets.emplace_back(make_tags_box(parent));
    widgets.emplace_back(make_url_box(parent));
    return widgets;
}
QWidget* DiscordWebhookUrl::make_enabled_box(QWidget& parent){
    QWidget* wrapper = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(wrapper);
    layout->setAlignment(Qt::AlignCenter);
    layout->setMargin(0);
    QCheckBox* box = new QCheckBox(&parent);
    box->setChecked(enabled);
    layout->addWidget(box);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [=](int){
            enabled = box->isChecked();
        }
    );
    return wrapper;
}
QWidget* DiscordWebhookUrl::make_label_box(QWidget& parent){
    QLineEdit* box = new QLineEdit(&parent);
    box->setText(label);
    box->setPlaceholderText("My test server");
    box->connect(
        box, &QLineEdit::textChanged,
        box, [=](const QString& line){
            label = line;
        }
    );
    return box;
}
QWidget* DiscordWebhookUrl::make_ping_box(QWidget& parent){
    QWidget* wrapper = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(wrapper);
    layout->setAlignment(Qt::AlignCenter);
    layout->setMargin(0);
    QCheckBox* box = new QCheckBox(&parent);
    box->setChecked(ping);
    layout->addWidget(box);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [=](int){
            ping = box->isChecked();
        }
    );
    return wrapper;
}
QWidget* DiscordWebhookUrl::make_tags_box(QWidget& parent){
    QLineEdit* box = new QLineEdit(&parent);
    box->setText(EventNotificationSettings::tags_to_str(tags));
//    box->setAlignment(Qt::AlignHCenter);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [&](const QString& text){
            tags = EventNotificationSettings::parse_tags(text);
        }
    );
    return box;
}
QWidget* DiscordWebhookUrl::make_url_box(QWidget& parent){
    QLineEdit* box = new QLineEdit(&parent);
    box->setText(url);
    box->setPlaceholderText("https://discord.com/api/webhooks/123456789012345678/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    box->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [=](const QString& line){
            url = line;
        }
    );
    return box;
}



QStringList DiscordWebhookUrlsFactory::make_header() const{
    QStringList list;
    list << "Enabled" << "Description" << "Allow Pings" << "Tags" << "Webhook URL";
    return list;
}
std::unique_ptr<EditableTableRow> DiscordWebhookUrlsFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new DiscordWebhookUrl());
}




DiscordWebhookSettingsOption::DiscordWebhookSettingsOption()
    : GroupOption("Discord Webhook Settings", true, false)
    , urls(
        "<b>Discord Webhook URLs:</b> Notifications are sent to all enabled URLs that share a tag with the event.",
        m_factory, true
    )
{
    PA_ADD_OPTION(urls);
}



}
}
