/*  Discord Integration Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCheckBox>
#include <QLineEdit>
#include "Common/Qt/QtJsonTools.h"
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

void DiscordIntegrationChannel::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    json_get_bool(enabled, obj, "Enabled");
    json_get_string(label, obj, "Label");
    json_get_bool(ping, obj, "Ping");
    QJsonArray array = json_get_array_nothrow(obj, "Tags");
    json_get_bool(allow_commands, obj, "Commands");
    tags.clear();
    for (const auto& tag : array){
        QString token = EventNotificationSettings::sanitize_tag(tag.toString());
        if (!token.isEmpty()){
            tags.emplace_back(std::move(token));
        }
    }
    json_get_string(channel_id, obj, "Channel");
}
QJsonValue DiscordIntegrationChannel::to_json() const{
    QJsonObject obj;
    obj["Enabled"] = enabled;
    obj["Label"] = label;
    obj["Ping"] = ping;
    QJsonArray array;
    for (const QString& tag : tags){
        array.append(tag);
    }
    obj.insert("Tags", array);
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
    box->setText(channel_id);
    box->setPlaceholderText("123456789012345678");
    box->connect(
        box, &QLineEdit::textChanged,
        box, [=](const QString& line){
            channel_id = line;
        }
    );
    return box;
}



QStringList DiscordIntegrationTableFactory::make_header() const{
    QStringList list;
    list << "Enabled" << "Description" << "Allow Pings" << "Tags" << "Allow Commands" << "Channel ID";
    return list;
}
std::unique_ptr<EditableTableRow> DiscordIntegrationTableFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new DiscordIntegrationChannel());
}



DiscordIntegrationTable::DiscordIntegrationTable()
    : EditableTableOption(
        "<b>Discord Channels:</b> Configure which channels to send notifications and accept commands in.",
        *this, true
    )
{}
void DiscordIntegrationTable::load_json(const QJsonValue& json){
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

std::vector<QString> DiscordIntegrationTable::command_channels() const{
    std::vector<QString> ret;
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
