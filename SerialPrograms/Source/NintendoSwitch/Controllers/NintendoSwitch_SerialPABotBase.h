/*  Nintendo Switch Controller (Serial PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_NintendoSwitch_Controller_SerialPABotBase_H
#define PokemonAutomation_NintendoSwitch_Controller_SerialPABotBase_H

#include "ClientSource/Connection/BotBase.h"
#include "NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




//  Temporary class for refactor.
class SwitchControllerSerialPABotBase :
    public SwitchController,
    public BotBaseController
{
public:
    using ContextType = SwitchControllerContext;

public:
    SwitchControllerSerialPABotBase(BotBaseController& serial)
        : m_serial(serial)
    {}

    virtual Logger& logger() override{
        return m_serial.logger();
    }


public:
    //  General Control

    virtual void wait_for_all(const Cancellable& cancellable) override;
    virtual void cancel_all(const Cancellable& cancellable) override;
    virtual void replace_on_next_command(const Cancellable& cancellable) override;


public:
    //  Basic Commands

    virtual void send_wait_for_pending(const Cancellable& cancellable) override;
    virtual void send_wait(const Cancellable& cancellable, uint16_t ticks) override;
    virtual void send_controller_state(
        const Cancellable& cancellable,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y,
        uint16_t ticks
    ) override;
    virtual void send_botbase_request(
        const Cancellable& cancellable,
        const BotBaseRequest& request
    ) override;
    virtual BotBaseMessage send_botbase_request_and_wait(
        const Cancellable& cancellable,
        const BotBaseRequest& request
    ) override;


public:
    //  Superscalar Commands (the "ssf" framework)

    virtual void send_buttons(
        const Cancellable& cancellable,
        Button button,
        uint16_t delay, uint16_t hold, uint8_t cooldown
    ) override;
    virtual void send_dpad(
        const Cancellable& cancellable,
        DpadPosition position,
        uint16_t delay, uint16_t hold, uint8_t cooldown
    ) override;
    virtual void send_left_joystick(
        const Cancellable& cancellable,
        uint8_t x, uint8_t y,
        uint16_t delay, uint16_t hold, uint8_t cooldown
    ) override;
    virtual void send_right_joystick(
        const Cancellable& cancellable,
        uint8_t x, uint8_t y,
        uint16_t delay, uint16_t hold, uint8_t cooldown
    ) override;


public:
    //  High speed RPCs.

    virtual void send_mash_button(
        const Cancellable& cancellable,
        Button button, uint16_t ticks
    ) override;
    virtual void send_mash_button(
        const Cancellable& cancellable,
        Button button0, Button button1, uint16_t ticks
    ) override;
    virtual void send_mash_AZs(
        const Cancellable& cancellable,
        uint16_t ticks
    ) override;
    virtual void send_system_scroll(
        const Cancellable& cancellable,
        DpadPosition direction, //  Diagonals not allowed.
        uint16_t delay, uint16_t hold, uint8_t cooldown
    ) override;




public:
    virtual State state() const override{
        return m_serial.state();
    }
    virtual size_t queue_limit() const override{
        return m_serial.queue_limit();
    }
    virtual void wait_for_all_requests(const Cancellable* cancelled = nullptr) override{
        m_serial.wait_for_all_requests(cancelled);
    }
    virtual bool try_stop_all_commands() override{
        return m_serial.try_stop_all_commands();
    }
    virtual void stop_all_commands() override{
        m_serial.stop_all_commands();
    }
    virtual bool try_next_command_interrupt() override{
        return m_serial.try_next_command_interrupt();
    }
    virtual void next_command_interrupt() override{
        return m_serial.next_command_interrupt();
    }
    virtual bool try_issue_request(
        const BotBaseRequest& request,
        const Cancellable* cancelled = nullptr
    ) override{
        return m_serial.try_issue_request(request, cancelled);
    }
    virtual void issue_request(
        const BotBaseRequest& request,
        const Cancellable* cancelled = nullptr
    ) override{
        m_serial.issue_request(request, cancelled);
    }
    virtual BotBaseMessage issue_request_and_wait(
        const BotBaseRequest& request,
        const Cancellable* cancelled = nullptr
    ) override{
        return m_serial.issue_request_and_wait(request, cancelled);
    }


private:
    BotBaseController& m_serial;

};




}
}
#endif
