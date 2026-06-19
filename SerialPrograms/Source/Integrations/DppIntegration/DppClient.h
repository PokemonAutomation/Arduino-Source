#pragma once
#ifndef DPP_CLIENT_H
#define DPP_CLIENT_H

#ifdef PA_DPP
#include <atomic>
#include <Integrations/DppIntegration/DppCommandHandler.h>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "CommonFramework/Notifications/MessageAttachment.h"
#include "Common/Cpp/Options/ButtonOption.h"

namespace PokemonAutomation{
    class JsonObject;
namespace Integration{
namespace DppClient{



class Client : protected DppCommandHandler::Handler, public ButtonListener{
public:
    Client() : m_is_connected(false) {}
    ~Client();
    static Client& instance();

    virtual void stop() override{
        disconnect();
        Handler::stop();
    }

public:
    bool is_initialized();
    bool is_running();

    void connect();
    void disconnect();
    void send_embed_dpp(
        bool should_ping,
        const Color& color,
        const std::vector<std::string>& tags,
        const JsonObject& json_obj,
        std::shared_ptr<PendingFileSend> file
    );

private:
    void run(const std::string& token);
    virtual void on_press() override{
        std::lock_guard<std::mutex> lg(m_register_lock);
        if (m_handler && m_handler->slash_commands_enabled && GlobalSettings::instance().DISCORD->integration.register_slash_button.is_enabled()){
            GlobalSettings::instance().DISCORD->integration.register_slash_button.set_enabled(false);
            log_dpp("Registering commands...", "Slash Command Registration", dpp::ll_info);
            m_handler->register_commands();
            log_dpp("Registered commands with Discord. You may have to wait or restart Discord for changes to take effect.", "Slash Command Registration", dpp::ll_info);
            GlobalSettings::instance().DISCORD->integration.register_slash_button.set_enabled(true);
            return;
        }

        log_dpp("Failed to register commands. Make sure the bot is connected and Slash commands are enabled.", "Slash Command Registration", dpp::ll_error);
    };

private:
    std::unique_ptr<dpp::cluster> m_bot = nullptr;
    std::unique_ptr<dpp::commandhandler> m_handler = nullptr;
    std::atomic<bool> m_is_connected;
    std::mutex m_client_lock;
    std::mutex m_register_lock;
    AsyncTask m_start_thread;
};



}
}
}
#endif

#endif
