#ifdef PA_DPP

#include <set>
#include <dpp/DPP_SilenceWarnings.h>
#include <dpp/dpp.h>
#include <Integrations/DppIntegration/DppClient.h>
#include <Integrations/DppIntegration/DppCommandHandler.h>
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"

using namespace dpp;
namespace PokemonAutomation{
namespace Integration{
namespace DppClient{


Client& Client::instance() {
    static Client client;
    return client;
}

bool Client::is_running() {
    return Client::m_is_connected;
}

void Client::connect() {
    std::lock_guard<std::mutex> lg(m_connect_lock);
    if (m_bot == nullptr && !m_is_connected) {
        DiscordSettingsOption& settings = GlobalSettings::instance().DISCORD;
        if (!Handler::check_if_empty(settings))
            return;

        std::string token = settings.integration.token;
        try {
            std::thread(&Client::run, this, token).detach();
            m_is_connected = true;
        }
        catch (std::exception& e) {
            Handler::log_dpp("DPP thew an exception: " + (std::string)e.what(), "connect()", ll_critical);
        }
    }
}

void Client::disconnect() {
    std::lock_guard<std::mutex> lg(m_connect_lock);
    if (m_bot != nullptr && m_is_connected) {
        try {
            m_bot->shutdown();
            m_bot = nullptr;
            m_is_connected = false;
        }
        catch (std::exception& e) {
            Handler::log_dpp("DPP thew an exception: " + (std::string)e.what(), "disconnect()", ll_critical);
        }
    }
}

void Client::send_embed_dpp(
    bool should_ping,
    const Color& color,
    const std::vector<std::string>& tags,
    const JsonObject& json_obj,
    std::shared_ptr<PendingFileSend> file
){
    DiscordSettingsOption& settings = GlobalSettings::instance().DISCORD;
    if (!settings.integration.enabled()){
        return;
    }
    if (!m_is_connected){
        return;
    }


    embed embed;
    {
        embed.set_title(*json_obj.get_string("title"));
        embed.set_color((int)((uint32_t)color & 0xffffff));

        auto fields = json_obj.get_array("fields");
        for (auto& field : *fields) {
            auto obj = field.get_object();
            embed.add_field(*obj->get_string("name"), *obj->get_string("value"));
        }
    }

    std::set<std::string> tag_set;
    for (const std::string& tag : tags) {
        tag_set.insert(to_lower(tag));
    }

    MessageBuilder builder(tags);
    const DiscordIntegrationTable& channels = settings.integration.channels;

    auto table = channels.copy_snapshot();
    for (auto& ch : table) {
        const Integration::DiscordIntegrationChannel& channel = *ch;
        if (((std::string)channel.tags_text).empty() || !channel.enabled) {
            continue;
        }
        if (!builder.should_send(EventNotificationOption::parse_tags(channel.tags_text))){
            continue;
        }

        Handler::send_message(
            *m_bot.get(), embed,
            channel.channel_id, std::chrono::seconds(channel.delay),
            builder.build_message(
                should_ping && channel.ping,
                settings.message.user_id,
                settings.message.message
            ),
            file
        );
    }
}

void Client::run(const std::string& token) {
    try {
        uint32_t intents = intents::i_default_intents | intents::i_guild_members;
        m_bot = std::make_unique<cluster>(token, intents);
        Handler::initialize(*m_bot.get());
        m_bot->start(false);
    }
    catch (std::exception& e) {
        Handler::log_dpp("DPP thew an exception: " + (std::string)e.what(), "run()", ll_critical);
        m_is_connected = false;
    }
}


}
}
}
#endif
