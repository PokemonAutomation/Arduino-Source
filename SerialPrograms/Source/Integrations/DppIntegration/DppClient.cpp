#ifdef PA_DPP

#include <set>
#include <dpp/DPP_SilenceWarnings.h>
#include <dpp/dpp.h>
#include <Integrations/DppIntegration/DppClient.h>
#include <Integrations/DppIntegration/DppCommandHandler.h>
#include "Common/Cpp/Json/JsonArray.h"
//#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/StringToolsQt.h"
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

void Client::send_message_dpp(
    bool should_ping,
    const Color& color,
    const std::vector<std::string>& tags,
    const JsonObject& json_obj,
    const std::string& message,
    std::shared_ptr<PendingFileSend> file) {
    if (m_is_connected) {
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

        DiscordSettingsOption& settings = GlobalSettings::instance().DISCORD;
        const DiscordIntegrationTable& channels = settings.integration.channels;
        std::vector<std::string> channel_vector;
        std::vector<std::string> message_vector;

        auto table = channels.copy_snapshot();
        for (auto& ch : table) {
            const Integration::DiscordIntegrationChannel& channel = *ch;
            if (((std::string)channel.tags_text).empty() || !channel.enabled) {
                continue;
            }

            bool send = false;
            for (const std::string& tag : EventNotificationOption::parse_tags(channel.tags_text)) {
                auto iter = tag_set.find(to_lower(tag));
                if (iter != tag_set.end()) {
                    channel_vector.emplace_back(channel.channel_id);
                    send = true;
                    break;
                }
            }

            if (!send) {
                continue;
            }

            if (std::atoll(((std::string)settings.message.user_id).c_str()) == 0) {
                should_ping = false;
            }

            std::string msg = "";
            if (should_ping && channel.ping) {
                msg += "<@" + (std::string)settings.message.user_id + ">";
            }

            std::string discord_message = settings.message.message;
            if (!discord_message.empty()) {
                if (!msg.empty()) {
                    msg += " ";
                }

                discord_message.erase(std::remove(discord_message.begin(), discord_message.end(), '@'), discord_message.end());
                msg += discord_message;
            }

            if (!message.empty()) {
                if (!msg.empty()) {
                    msg += " ";
                }
                msg += message;
            }

            message_vector.emplace_back(msg);
            Handler::log_dpp("Sending message...", "send_message_dpp()", ll_info);
            Handler::send_message(*m_bot.get(), embed, channel.channel_id, std::chrono::seconds(channel.delay), msg, file);
        }
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
