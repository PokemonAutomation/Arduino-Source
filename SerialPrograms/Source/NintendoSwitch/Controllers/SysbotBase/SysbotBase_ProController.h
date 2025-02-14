/*  Nintendo Switch Pro Controller (SysbotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_NintendoSwitch_ProController_SysbotBase_H
#define PokemonAutomation_NintendoSwitch_ProController_SysbotBase_H

#include <mutex>
#include <condition_variable>
#include "NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProControllerWithScheduler.h"
#include "SysbotBase_Connection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class ProController_SysbotBase : public ProControllerWithScheduler{
public:
    using ContextType = ProControllerContext;

    static constexpr size_t QUEUE_SIZE = 8;
    static constexpr std::chrono::microseconds EARLY_WAKE_SPIN = std::chrono::microseconds(2000);


public:
    ProController_SysbotBase(
        Logger& logger,
        SysbotBase::SysbotBaseNetwork_Connection& connection,
        const ControllerRequirements& requirements
    );
    ~ProController_SysbotBase();

    virtual bool is_ready() const override{
        return m_connection.is_ready() && m_error_string.empty();
    }
    virtual std::string error_string() const override{
        return m_error_string;
    }


public:
    //  General Control

    virtual void wait_for_all(const Cancellable* cancellable) override;
    virtual void cancel_all_commands() override;
    virtual void replace_on_next_command() override;


public:
    //  Basic Commands

    virtual void issue_controller_state(
        const Cancellable* cancellable,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y,
        Milliseconds duration
    ) override;
    virtual void send_botbase_request(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    ) override;
    virtual BotBaseMessage send_botbase_request_and_wait(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    ) override;


private:
    void send_diff(
        const SwitchControllerState& old_state,
        const SwitchControllerState& new_state
    );
    void thread_body();


private:
    SysbotBase::SysbotBaseNetwork_Connection& m_connection;

    std::string m_error_string;

    std::atomic<bool> m_stopping;
    std::atomic<bool> m_replace_on_next;

    struct Command{
        SwitchControllerState state;
        Milliseconds duration;
    };
    std::deque<Command> m_command_queue;
    WallClock m_queue_start_time;
    bool m_is_active;

    std::mutex m_lock;
    std::condition_variable m_cv;
    std::thread m_dispatch_thread;
};




}
}
#endif
