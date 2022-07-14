/*  Discord Integration Table
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
#include "DiscordIntegrationTable.h"

namespace PokemonAutomation{
namespace Integration{


DiscordIntegrationChannel::DiscordIntegrationChannel()
    : enabled(true)
    , ping(true)
    , tags({"Notifs", "Showcase", "LiveHost"})
    , allow_commands(true)
{}

void DiscordIntegrationChannel::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    obj->read_boolean(enabled, "Enabled");
    obj->read_string(label, "Label");
    obj->read_boolean(ping, "Label");
    const JsonArray* array = obj->get_array("Tags");
    if (array != nullptr){
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
    obj->read_boolean(allow_commands, "Commands");
    obj->read_string(channel_id, "Channel");
}
JsonValue DiscordIntegrationChannel::to_json() const{
    JsonObject obj;
    obj["Enabled"] = enabled;
    obj["Label"] = label;
    obj["Ping"] = ping;
    JsonArray array;
    for (const std::string& tag : tags){
        array.push_back(tag);
    }
    obj["Tags"] = std::move(array);
    obj["Commands"] = allow_commands;
    obj["Channel"] = channel_id;
    return obj;
}
std::unique_ptr<EditableTableRow> DiscordIntegrationChannel::clone() const{
    return std::unique_ptr<EditableTableRow>(new DiscordIntegrationChannel(*this));
}

std::vector<QWidget*> DiscordIntegrationChannel::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets;
    widgets.emplace_back(make_enabled_box(parent));
    widgets.emplace_back(make_label_box(parent));
    widgets.emplace_back(make_ping_box(parent));
    widgets.emplace_back(make_tags_box(parent));
    widgets.emplace_back(make_commands_box(parent));
    widgets.emplace_back(make_channel_box(parent));
    return widgets;
}
QWidget* DiscordIntegrationChannel::make_enabled_box(QWidget& parent){
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
QWidget* DiscordIntegrationChannel::make_label_box(QWidget& parent){
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
QWidget* DiscordIntegrationChannel::make_ping_box(QWidget& parent){
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
QWidget* DiscordIntegrationChannel::make_tags_box(QWidget& parent){
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
QWidget* DiscordIntegrationChannel::make_commands_box(QWidget& parent){
    QWidget* wrapper = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(wrapper);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    QCheckBox* box = new QCheckBox(&parent);
    box->setChecked(allow_commands);
    layout->addWidget(box);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [=](int){
            allow_commands = box->isChecked();
        }
    );
    return wrapper;
}

QWidget* DiscordIntegrationChannel::make_channel_box(QWidget& parent){
    QLineEdit* box = new QLineEdit(&parent);
    box->setText(QString::fromStdString(channel_id));
    box->setPlaceholderText("123456789012345678");
    box->connect(
        box, &QLineEdit::textChanged,
        box, [=](const QString& line){
            channel_id = line.toStdString();
        }
    );
    return box;
}



std::vector<std::string> DiscordIntegrationTableFactory::make_header() const{
    return std::vector<std::string>{
        "Enabled",
        "Description",
        "Allow Pings",
        "Tags",
        "Allow Commands",
        "Channel ID",
    };
}
std::unique_ptr<EditableTableRow> DiscordIntegrationTableFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new DiscordIntegrationChannel());
}



DiscordIntegrationTable::DiscordIntegrationTable()
    : EditableTableOption(
        "<b>Discord Channels:</b> Configure which channels to send notifications and accept commands in.",
        *this
    )
{}
void DiscordIntegrationTable::load_json(const JsonValue& json){
    EditableTableOption::load_json(json);
    if (size() == 0){
        std::unique_ptr<DiscordIntegrationChannel> row;

        row.reset(new DiscordIntegrationChannel());
        row->label = "Main Channel";
        add_row(0, std::move(row));
    }
}

size_t DiscordIntegrationTable::size() const{
    return EditableTableOption::size();
}
const DiscordIntegrationChannel& DiscordIntegrationTable::operator[](size_t index) const{
    return static_cast<const DiscordIntegrationChannel&>(EditableTableOption::operator[](index));
}

std::vector<std::string> DiscordIntegrationTable::command_channels() const{
    std::vector<std::string> ret;
    for (size_t c = 0; c < size(); c++){
        const DiscordIntegrationChannel& channel = (*this)[c];
        if (channel.enabled && channel.allow_commands){
            ret.emplace_back(channel.channel_id);
        }
    }
    return ret;
}








}
}
