#pragma once
#ifndef DPP_HANDLER_H
#define DPP_HANDLER_H

#include <dpp/dpp.h>
#include <Integrations/DppIntegration/DppUtility.h>
#include "Common/Cpp/Concurrency/ScheduledTaskRunner.h"
#include "CommonFramework/Notifications/MessageAttachment.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "Integrations/DiscordSettingsOption.h"

namespace PokemonAutomation{
namespace Integration{
namespace DppCommandHandler{



class Handler : DppUtility::Utility{
public:
    Handler()
        : m_queue(GlobalThreadPools::unlimited_normal())
    {}
    ~Handler(){
        m_queue.stop();
    }

    virtual void stop(){
        m_queue.stop();
    }

private:
    struct SlashCommand{
        dpp::slashcommand command;
        std::function<void(const dpp::slashcommand_t&)> func;
    };

    std::mutex m_count_lock;
    static dpp::user owner;
    static Color color;
    ScheduledTaskRunner m_queue;

protected:
    void initialize(dpp::cluster& bot, dpp::commandhandler& handler);
    bool check_if_empty(const DiscordSettingsOption& settings);
    void log_dpp(const std::string& message, const std::string& identity, const dpp::loglevel& ll);
    void send_message(
        dpp::cluster& bot,
        dpp::embed& embed,
        const std::string& channel,
        std::chrono::milliseconds delay,
        const std::string& msg,
        std::shared_ptr<PendingFileSend> file
    );

private:
    void create_unified_commands(dpp::commandhandler& handler);
    void update_response(
        const dpp::command_source& src,
        dpp::embed& embed,
        const std::string& msg,
        std::shared_ptr<PendingFileSend> file
    );
};




}
}
}
#endif
