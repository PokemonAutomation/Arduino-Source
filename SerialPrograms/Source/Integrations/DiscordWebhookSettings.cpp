/*  Discord Webhook Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "DiscordWebhookSettings.h"

namespace PokemonAutomation{
namespace Integration{



DiscordWebhookUrl::DiscordWebhookUrl()
    : enabled(LockWhileRunning::LOCKED, true)
    , label(false, LockWhileRunning::LOCKED, "", "My test server")
    , ping(LockWhileRunning::LOCKED, true)
    , tags_text(false, LockWhileRunning::LOCKED, "Notifs, Showcase, LiveHost", "")
    , url(true, LockWhileRunning::LOCKED, "", "https://discord.com/api/webhooks/123456789012345678/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
{
    //  Keep the old JSON tags for backwards compatibility.
    add_option(enabled, "Enabled");
    add_option(label, "Label");
    add_option(ping, "Ping");
    add_option(tags_text, "Tags");
    add_option(url, "URL");
}
std::unique_ptr<EditableTableRow> DiscordWebhookUrl::clone() const{
    std::unique_ptr<DiscordWebhookUrl> ret(new DiscordWebhookUrl());
    ret->enabled = (bool)enabled;
    ret->label.set(label);
    ret->ping = (bool)ping;
    ret->tags_text.set(tags_text);
    ret->url.set(url);
    return ret;
}
void DiscordWebhookUrl::load_json(const JsonValue& json){
    EditableTableRow::load_json(json);
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



DiscordWebhookSettingsTable::DiscordWebhookSettingsTable()
    : EditableTableOption_t<DiscordWebhookUrl>(
        "<b>Discord Webhook URLs:</b> Notifications are sent to all enabled URLs that share a tag with the event.",
        LockWhileRunning::LOCKED
    )
{}
std::vector<std::string> DiscordWebhookSettingsTable::make_header() const{
    return std::vector<std::string>{
        "Enabled",
        "Description",
        "Allow Pings",
        "Tags",
        "Webhook URL",
    };
}




DiscordWebhookSettingsOption::DiscordWebhookSettingsOption()
    : GroupOption("Discord Webhook Settings", LockWhileRunning::LOCKED, true, false)
{
    PA_ADD_OPTION(urls);
}



}
}
