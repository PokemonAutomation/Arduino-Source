/*  DPP Command Handler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Integrations_DppCommandHandler_H
#define PokemonAutomation_Integrations_DppCommandHandler_H

#include <Integrations/DppIntegration/DppUtility.h>
#include "Common/Cpp/Concurrency/ScheduledTaskRunner.h"
#include "CommonFramework/Notifications/MessageAttachment.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "Integrations/IntegrationsAPI.h"
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
    static uint8_t get_min_parameters(const dpp::parameter_registration_t& params);

    void add_command_hi(dpp::commandhandler& handler);
    void add_command_ping(dpp::commandhandler& handler);
    void add_command_about(dpp::commandhandler& handler);
    void add_command_help(dpp::commandhandler& handler);
    void add_command_status(dpp::commandhandler& handler);
    void add_command_screenshot(dpp::commandhandler& handler, bool full_version);
    void add_command_start(dpp::commandhandler& handler);
    void add_command_stop(dpp::commandhandler& handler);
    void add_command_resetcamera(dpp::commandhandler& handler);
    void add_command_resetcontroller(dpp::commandhandler& handler);
    void add_command_click(dpp::commandhandler& handler, bool full_version);
    void add_command_joystick(dpp::commandhandler& handler, bool full_version, JoystickSide side);


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
