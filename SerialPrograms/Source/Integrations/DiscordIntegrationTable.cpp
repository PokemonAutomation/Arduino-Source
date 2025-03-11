/*  Discord Integration Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "DiscordIntegrationTable.h"

namespace PokemonAutomation{
namespace Integration{



DiscordIntegrationChannel::DiscordIntegrationChannel(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , enabled(LockMode::UNLOCK_WHILE_RUNNING, true)
    , label(false, LockMode::UNLOCK_WHILE_RUNNING, "", "My test server")
    , ping(LockMode::UNLOCK_WHILE_RUNNING, true)
    , tags_text(false, LockMode::UNLOCK_WHILE_RUNNING, "Notifs, Showcase, LiveHost", "")
    , allow_commands(LockMode::UNLOCK_WHILE_RUNNING, true)
    , delay(LockMode::UNLOCK_WHILE_RUNNING, 0, 0, 10)
    , channel_id(false, LockMode::UNLOCK_WHILE_RUNNING, "", "123456789012345678")
{
    //  Keep the old JSON tags for backwards compatibility.
    add_option(enabled, "Enabled");
    add_option(label, "Label");
    add_option(ping, "Ping");
    add_option(tags_text, "Tags");
    add_option(allow_commands, "Commands");
    add_option(delay, "Delay");
    add_option(channel_id, "Channel");
}
std::unique_ptr<EditableTableRow> DiscordIntegrationChannel::clone() const{
    std::unique_ptr<DiscordIntegrationChannel> ret(new DiscordIntegrationChannel(parent()));
    ret->enabled = (bool)enabled;
    ret->label.set(label);
    ret->ping = (bool)ping;
    ret->tags_text.set(tags_text);
    ret->allow_commands = (bool)allow_commands;
    ret->delay.set(delay);
    ret->channel_id.set(channel_id);
    return ret;
}
void DiscordIntegrationChannel::load_json(const JsonValue& json){
    EditableTableRow::load_json(json);
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    //  Load the old tags format.
    const JsonArray* array = obj->get_array("Tags");
    if (array){
        std::string tags;
        for (const auto& tag : *array){
            const std::string* str = tag.to_string();
            if (str == nullptr){
                continue;
            }
            std::string token = EventNotificationOption::sanitize_tag(*str);
            if (!token.empty()){
                if (!tags.empty()){
                    tags += ", ";
                }
                tags += token;
            }
        }
        tags_text.set(std::move(tags));
    }
}



DiscordIntegrationTable::DiscordIntegrationTable()
    : EditableTableOption_t<DiscordIntegrationChannel>(
        "<b>Discord Channels:</b> Configure which channels to send notifications and accept commands in.",
        LockMode::UNLOCK_WHILE_RUNNING
    )
{}
std::vector<std::string> DiscordIntegrationTable::make_header() const{
    return std::vector<std::string>{
        "Enabled",
        "Description",
        "Allow Pings",
        "Tags",
        "Allow Commands",
        "Delay (seconds)",
        "Channel ID",
    };
}
std::vector<std::string> DiscordIntegrationTable::command_channels() const{
    std::vector<std::unique_ptr<DiscordIntegrationChannel>> table = copy_snapshot();
    std::vector<std::string> ret;
    for (const auto& channel : table){
        if (channel->enabled && channel->allow_commands){
            ret.emplace_back(channel->channel_id);
        }
    }
    return ret;
}









}
}
