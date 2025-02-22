/*  Nintendo Switch Pro Controller (SysbotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_NintendoSwitch_ProController_SysbotBase_H
#define PokemonAutomation_NintendoSwitch_ProController_SysbotBase_H

#include <condition_variable>
#include "Common/Cpp/Containers/CircularBuffer.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProControllerWithScheduler.h"
#include "SysbotBase_Connection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class ProController_SysbotBase : public ProControllerWithScheduler{
public:
    using ContextType = ProControllerContext;

    static constexpr size_t QUEUE_SIZE = 4;


public:
    ProController_SysbotBase(
        Logger& logger,
        SysbotBase::TcpSysbotBase_Connection& connection,
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
    virtual void cancel_all_commands() override;
    virtual void replace_on_next_command() override;

    virtual void wait_for_all(const Cancellable* cancellable) override;

    virtual void send_botbase_request(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    ) override;
    virtual BotBaseMessage send_botbase_request_and_wait(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    ) override;


private:
    virtual void push_state(const Cancellable* cancellable, WallDuration duration) override;

    void send_diff(
        const SwitchControllerState& old_state,
        const SwitchControllerState& new_state
    );
    void thread_body();


private:
    SysbotBase::TcpSysbotBase_Connection& m_connection;

    std::string m_error_string;

    std::atomic<bool> m_stopping;
    bool m_replace_on_next;

    struct Command{
        SwitchControllerState state;
        Milliseconds duration;
    };
    CircularBuffer<Command> m_command_queue;
    WallClock m_queue_start_time;
    bool m_is_active;

    std::condition_variable m_cv;
    std::thread m_dispatch_thread;
};




}
}
#endif
