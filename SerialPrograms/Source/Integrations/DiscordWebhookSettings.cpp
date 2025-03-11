/*  Discord Webhook Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "DiscordWebhookSettings.h"

namespace PokemonAutomation{
namespace Integration{



DiscordWebhookUrl::DiscordWebhookUrl(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , enabled(LockMode::LOCK_WHILE_RUNNING, true)
    , label(false, LockMode::LOCK_WHILE_RUNNING, "", "My test server")
    , ping(LockMode::LOCK_WHILE_RUNNING, true)
    , tags_text(false, LockMode::LOCK_WHILE_RUNNING, "Notifs, Showcase, LiveHost", "")
    , delay(LockMode::LOCK_WHILE_RUNNING, 0, 0, 10)
    , url(true, LockMode::LOCK_WHILE_RUNNING, "", "https://discord.com/api/webhooks/123456789012345678/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
{
    //  Keep the old JSON tags for backwards compatibility.
    add_option(enabled, "Enabled");
    add_option(label, "Label");
    add_option(ping, "Ping");
    add_option(tags_text, "Tags");
    add_option(delay, "Delay");
    add_option(url, "URL");
}
std::unique_ptr<EditableTableRow> DiscordWebhookUrl::clone() const{
    std::unique_ptr<DiscordWebhookUrl> ret(new DiscordWebhookUrl(parent()));
    ret->enabled = (bool)enabled;
    ret->label.set(label);
    ret->ping = (bool)ping;
    ret->tags_text.set(tags_text);
    ret->delay.set(delay);
    ret->url.set(url);
    return ret;
}
void DiscordWebhookUrl::load_json(const JsonValue& json){
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



DiscordWebhookSettingsTable::DiscordWebhookSettingsTable()
    : EditableTableOption_t<DiscordWebhookUrl>(
        "<b>Discord Webhook URLs:</b> Notifications are sent to all enabled URLs that share a tag with the event.",
        LockMode::LOCK_WHILE_RUNNING
    )
{}
std::vector<std::string> DiscordWebhookSettingsTable::make_header() const{
    return std::vector<std::string>{
        "Enabled",
        "Description",
        "Allow Pings",
        "Tags",
        "Delay (seconds)",
        "Webhook URL",
    };
}




DiscordWebhookSettingsOption::DiscordWebhookSettingsOption()
    : GroupOption(
        "Discord Webhook Settings",
        LockMode::LOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_ENABLED, false
    )
    , sends_per_second(
        "<b>Rate Limit:</b><br>Maximum number of sends per second.",
        LockMode::LOCK_WHILE_RUNNING, 2, 1
    )
{
    PA_ADD_OPTION(urls);
    PA_ADD_OPTION(sends_per_second);
}



}
}
