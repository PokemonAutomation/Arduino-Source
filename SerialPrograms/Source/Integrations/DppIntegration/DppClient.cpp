#ifdef PA_DPP

#include <set>
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "Integrations/DppIntegration/DppClient.h"
#include "Integrations/DppIntegration/DppCommandHandler.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Integration{
namespace DppClient{

using namespace dpp;


Client& Client::instance(){
    static Client client;
    return client;
}



Client::~Client(){
    stop();
}

void Client::stop(){
    if (m_stopped){
        return;
    }
    m_stopped = true;
    GlobalSettings::instance().DISCORD->integration.register_slash_button.remove_listener(*this);
    disconnect();
    Handler::stop();
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
        auto cmd_type = GlobalSettings::instance().DISCORD->integration.command_type.get();
        uint32_t intents = intents::i_default_intents;
        if (cmd_type == DiscordIntegrationSettingsOption::CommandType::MessageCommands){
            intents = intents::i_default_intents | intents::i_message_content;
        }

        try{
            m_bot = std::make_unique<cluster>(token, intents);
            m_handler = std::make_unique<commandhandler>(m_bot.get(), false);
            m_bot->cache_policy = {
                cache_policy_setting_t::cp_lazy,
                cache_policy_setting_t::cp_lazy,
                cache_policy_setting_t::cp_aggressive
            };
            m_start_thread = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
                [&, this]{ run(token); }
            );
        }catch (std::exception& e){
            Handler::log_dpp("DPP thew an exception: " + (std::string)e.what(), "connect()", ll_critical);
        }
    }
}

void Client::disconnect(){
    std::lock_guard<std::mutex> lg(m_client_lock);
//    cout << "Client::disconnect()" << endl;

    m_start_thread.wait_and_ignore_exceptions();

    if (m_bot == nullptr || !m_is_connected.load(std::memory_order_relaxed)){
        return;
    }
    try{
        m_bot->shutdown();
        m_handler.reset();
        m_bot.reset();
        m_is_connected.store(false, std::memory_order_release);
    }catch (std::exception& e){
        Handler::log_dpp("DPP thew an exception: " + (std::string)e.what(), "disconnect()", ll_critical);
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
        m_is_connected.store(true, std::memory_order_release);
        GlobalSettings::instance().DISCORD->integration.register_slash_button.add_listener(*this);
    }catch (std::exception& e){
        Handler::log_dpp("DPP thew an exception: " + (std::string)e.what(), "run()", ll_critical);
        m_handler.reset();
        m_bot.reset();
        m_is_connected.store(false, std::memory_order_release);
    }

//    cout << "Client::run() - ending" << endl;
}
void Client::on_press(ButtonCell& button){
    std::lock_guard<std::mutex> lg(m_register_lock);
    if (m_handler && m_handler->slash_commands_enabled){
        GlobalSettings::instance().DISCORD->integration.register_slash_button.set_enabled(false);
        log_dpp("Registering commands...", "Slash Command Registration", dpp::ll_info);
        m_handler->register_commands();
        log_dpp(
            "Registered commands with Discord. You may have to wait or restart Discord for changes to take effect.",
            "Slash Command Registration",
            dpp::ll_info
        );
        GlobalSettings::instance().DISCORD->integration.register_slash_button.set_enabled(true);
        return;
    }

    log_dpp(
        "Failed to register commands. Make sure the bot is connected and Slash commands are enabled.",
        "Slash Command Registration",
        dpp::ll_error
    );
};


}
}
}
#endif
