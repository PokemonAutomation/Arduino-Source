/*  Discord Webhook Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "DiscordWebhookSettings.h"

namespace PokemonAutomation{
namespace Integration{



DiscordWebhookUrl::DiscordWebhookUrl()
    : enabled(true)
    , ping(true)
    , tags({"Notifs", "Showcase", "LiveHost"})
{}

void DiscordWebhookUrl::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    obj->read_boolean(enabled, "Enabled");
    {
        std::string str;
        if (obj->read_string(str, "Label")){
            label = str;
        }
    }
    obj->read_boolean(ping, "Ping");
    const JsonArray* array = obj->get_array("Tags");
    if (array){
        tags.clear();
        for (const auto& tag : *array){
            const std::string* str = tag.get_string();
            if (str == nullptr){
                continue;
            }
            std::string token = EventNotificationSettings::sanitize_tag(*str);
            if (!token.empty()){
                tags.emplace_back(std::move(token));
            }
        }
    }
    {
        std::string str;
        if (obj->read_string(str, "URL")){
            url = str;
        }
    }
}
JsonValue DiscordWebhookUrl::to_json() const{
    JsonObject obj;
    obj["Enabled"] = enabled;
    obj["Label"] = label;
    obj["Ping"] = ping;
    JsonArray array;
    for (const std::string& tag : tags){
        array.push_back(tag);
    }
    obj["Tags"] = std::move(array);
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
    layout->setContentsMargins(0, 0, 0, 0);
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
    box->setText(QString::fromStdString(label));
    box->setPlaceholderText("My test server");
    box->connect(
        box, &QLineEdit::textChanged,
        box, [=](const QString& line){
            label = line.toStdString();
        }
    );
    return box;
}
QWidget* DiscordWebhookUrl::make_ping_box(QWidget& parent){
    QWidget* wrapper = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(wrapper);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
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
    box->setText(QString::fromStdString(EventNotificationSettings::tags_to_str(tags)));
//    box->setAlignment(Qt::AlignHCenter);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [&](const QString& text){
            tags = EventNotificationSettings::parse_tags(text.toStdString());
        }
    );
    return box;
}
QWidget* DiscordWebhookUrl::make_url_box(QWidget& parent){
    QLineEdit* box = new QLineEdit(&parent);
    box->setText(QString::fromStdString(url));
    box->setPlaceholderText("https://discord.com/api/webhooks/123456789012345678/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    box->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [=](const QString& line){
            url = line.toStdString();
        }
    );
    return box;
}



std::vector<std::string> DiscordWebhookUrlsFactory::make_header() const{
    return std::vector<std::string>{
        "Enabled",
        "Description",
        "Allow Pings",
        "Tags",
        "Webhook URL",
    };
}
std::unique_ptr<EditableTableRow> DiscordWebhookUrlsFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new DiscordWebhookUrl());
}




DiscordWebhookSettingsOption::DiscordWebhookSettingsOption()
    : GroupOption("Discord Webhook Settings", true, false)
    , urls(
        "<b>Discord Webhook URLs:</b> Notifications are sent to all enabled URLs that share a tag with the event.",
        m_factory
    )
{
    PA_ADD_OPTION(urls);
}



}
}
