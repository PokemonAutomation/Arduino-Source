/*  Discord Integration Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "DiscordIntegrationTable.h"

namespace PokemonAutomation{
namespace Integration{



DiscordIntegrationChannel::DiscordIntegrationChannel()
    : enabled(true)
    , label(false, "", "My test server")
    , ping(true)
    , tags_text(false, "Notifs, Showcase, LiveHost", "")
    , allow_commands(true)
    , channel_id(false, "", "123456789012345678")
{
    //  Keep the old JSON tags for backwards compatibility.
    add_option(enabled, "Enabled");
    add_option(label, "Label");
    add_option(ping, "Ping");
    add_option(tags_text, "Tags");
    add_option(allow_commands, "Commands");
    add_option(channel_id, "Channel");
}
std::unique_ptr<EditableTableRow2> DiscordIntegrationChannel::clone() const{
    std::unique_ptr<DiscordIntegrationChannel> ret(new DiscordIntegrationChannel());
    ret->enabled = (bool)enabled;
    ret->label.set(label);
    ret->ping = (bool)ping;
    ret->tags_text.set(tags_text);
    ret->allow_commands = (bool)allow_commands;
    ret->channel_id.set(channel_id);
    return ret;
}
void DiscordIntegrationChannel::load_json(const JsonValue& json){
    EditableTableRow2::load_json(json);
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }

    //  Load the old tags format.
    const JsonArray* array = obj->get_array("Tags");
    if (array){
        std::string tags;
        for (const auto& tag : *array){
            const std::string* str = tag.get_string();
            if (str == nullptr){
                continue;
            }
            std::string token = EventNotificationSettings::sanitize_tag(*str);
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
    : EditableTableOption2<DiscordIntegrationChannel>(
        "<b>Discord Channels:</b> Configure which channels to send notifications and accept commands in."
    )
{}
std::vector<std::string> DiscordIntegrationTable::make_header() const{
    return std::vector<std::string>{
        "Enabled",
        "Description",
        "Allow Pings",
        "Tags",
        "Allow Commands",
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
