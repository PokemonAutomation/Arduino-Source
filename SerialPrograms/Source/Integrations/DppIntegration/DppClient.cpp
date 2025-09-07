#ifdef PA_DPP

#include <set>
#include <dpp/dpp.h>
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "Integrations/DppIntegration/DppClient.h"
#include "Integrations/DppIntegration/DppCommandHandler.h"

using namespace dpp;
namespace PokemonAutomation{
namespace Integration{
namespace DppClient{


Client& Client::instance(){
    static Client client;
    return client;
}

bool Client::is_initialized(){
    std::lock_guard<std::mutex> lg(m_client_lock);
    return m_bot != nullptr;
}
bool Client::is_running(){
    return m_is_connected.load(std::memory_order_acquire);
}

void Client::connect(){
    std::lock_guard<std::mutex> lg(m_client_lock);
    if (m_bot == nullptr && !m_is_connected.load(std::memory_order_relaxed)){
        DiscordSettingsOption& settings = GlobalSettings::instance().DISCORD;
        if (!Handler::check_if_empty(settings))
            return;

        std::string token = settings.integration.token;
        uint32_t intents = intents::i_default_intents | intents::i_guild_members | intents::i_message_content;
        try{
            m_bot = std::make_unique<cluster>(token, intents);
            m_handler = std::make_unique<commandhandler>(m_bot.get(), false);
            m_bot->cache_policy = { cache_policy_setting_t::cp_lazy, cache_policy_setting_t::cp_lazy, cache_policy_setting_t::cp_aggressive };
            std::thread(&Client::run, this, token).detach();
        }catch (std::exception& e){
            Handler::log_dpp("DPP thew an exception: " + (std::string)e.what(), "connect()", ll_critical);
        }
    }
}

void Client::disconnect(){
    std::lock_guard<std::mutex> lg(m_client_lock);
    if (m_bot != nullptr && m_is_connected.load(std::memory_order_relaxed)){
        try{
            m_bot->shutdown();
            m_handler.reset();
            m_bot.reset();
            m_is_connected.store(false, std::memory_order_release);
        }catch (std::exception& e){
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
    std::lock_guard<std::mutex> lg(m_client_lock);
    DiscordSettingsOption& settings = GlobalSettings::instance().DISCORD;
    if (!settings.integration.enabled()){
        return;
    }
    if (!m_is_connected.load(std::memory_order_relaxed)){
        return;
    }


    embed embed;
    {
        embed.set_title(*json_obj.get_string("title"));
        embed.set_color((int)((uint32_t)color & 0xffffff));

        auto fields = json_obj.get_array("fields");
        for (auto& field : *fields){
            auto obj = field.to_object();
            embed.add_field(*obj->get_string("name"), *obj->get_string("value"));
        }
    }

    std::set<std::string> tag_set;
    for (const std::string& tag : tags){
        tag_set.insert(to_lower(tag));
    }

    MessageBuilder builder(tags);
    const DiscordIntegrationTable& channels = settings.integration.channels;

    auto table = channels.copy_snapshot();
    for (auto& ch : table){
        const Integration::DiscordIntegrationChannel& channel = *ch;
        if (((std::string)channel.tags_text).empty() || !channel.enabled){
            continue;
        }
        if (!builder.should_send(EventNotificationOption::parse_tags(channel.tags_text))){
            continue;
        }

        std::chrono::seconds delay(channel.delay);
        Handler::send_message(
            *m_bot.get(), embed,
            channel.channel_id,
            delay,
            builder.build_message(
                delay,
                should_ping && channel.ping,
                settings.message.user_id,
                settings.message.message
            ),
            file
        );
    }
}

void Client::run(const std::string& token){
    std::lock_guard<std::mutex> lg(m_client_lock);
    if (!m_bot){
        Handler::log_dpp("DPP has been disconnected.", "run()", ll_warning);
        return;
    }
    try{
        Handler::initialize(*m_bot.get(), *m_handler.get());
        m_bot->set_websocket_protocol(websocket_protocol_t::ws_etf);
        m_bot->start(st_return);
        m_bot->set_presence(
            presence(
                presence_status::ps_online,
                activity_type::at_game,
                (std::string)GlobalSettings::instance().DISCORD->integration.game_status
            )
        );
        m_is_connected.store(true, std::memory_order_release);
    }catch (std::exception& e){
        Handler::log_dpp("DPP thew an exception: " + (std::string)e.what(), "run()", ll_critical);
        m_handler.reset();
        m_bot.reset();
        m_is_connected.store(false, std::memory_order_release);
    }
}


}
}
}
#endif
