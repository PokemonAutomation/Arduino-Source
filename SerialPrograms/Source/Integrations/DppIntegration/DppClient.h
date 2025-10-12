#pragma once
#ifndef DPP_CLIENT_H
#define DPP_CLIENT_H

#ifdef PA_DPP
#include <atomic>
#include "DppCommandHandler.h"
#include "CommonFramework/Notifications/MessageAttachment.h"

namespace PokemonAutomation{
    class JsonObject;
namespace Integration{
namespace DppClient{



class Client : protected DppCommandHandler::Handler{
public:
    Client() : m_is_connected(false) {}
    ~Client();
    static Client& instance();

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

private:
    std::unique_ptr<dpp::cluster> m_bot = nullptr;
    std::unique_ptr<dpp::commandhandler> m_handler = nullptr;
    std::atomic<bool> m_is_connected;
    std::mutex m_client_lock;
    std::thread m_start_thread;
};



}
}
}
#endif

#endif
