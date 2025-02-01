/*  Nintendo Switch Controller (Serial PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_NintendoSwitch_Controller_SerialPABotBase_H
#define PokemonAutomation_NintendoSwitch_Controller_SerialPABotBase_H

#include <QSerialPortInfo>
#include "ClientSource/Connection/BotBase.h"
#include "Controllers/SuperscalarScheduler.h"
#include "Controllers/ControllerDescriptor.h"
#include "Controllers/ControllerConnection.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Handle.h"
#include "NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class SwitchController_SerialPABotBase_Descriptor : public ControllerDescriptor{
public:
    static const char* TYPENAME;

public:
    SwitchController_SerialPABotBase_Descriptor() = default;
    SwitchController_SerialPABotBase_Descriptor(const QSerialPortInfo& info)
        : m_port(info)
    {}

    const QSerialPortInfo& port() const{
        return m_port;
    }

    virtual bool operator==(const ControllerDescriptor& x) const override;
    virtual const char* type_name() const override;
    virtual std::string display_name() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual std::unique_ptr<ControllerConnection> open(
        Logger& logger,
        const ControllerRequirements& requirements
    ) const override;

private:
    QSerialPortInfo m_port;
};




template <typename Type>
PA_FORCE_INLINE Type milliseconds_to_ticks_8ms(Type milliseconds){
    return milliseconds / 8 + (milliseconds % 8 + 7) / 8;
}





struct SwitchButton_Dpad : public ExecutionResource{
    DpadPosition position;
};
struct SwitchButton_Joystick : public ExecutionResource{
    uint8_t x;
    uint8_t y;
};

struct SwitchControllerState{
    ExecutionResource m_buttons[14];
    SwitchButton_Dpad m_dpad;
    SwitchButton_Joystick m_left_joystick;
    SwitchButton_Joystick m_right_joystick;
};





class SwitchController_SerialPABotBase :
    public ControllerConnection,
    public SwitchController,
    private SwitchControllerState,
    private SuperscalarScheduler
{
public:
    using ContextType = SwitchControllerContext;

public:
    SwitchController_SerialPABotBase(
        Logger& logger,
        const SwitchController_SerialPABotBase_Descriptor& descriptor,
        const ControllerRequirements& requirements
    );

    virtual Logger& logger() override{
        return m_serial.logger();
    }


public:
    //  General Control

    virtual void wait_for_all(const Cancellable* cancellable) override;
    virtual void cancel_all(const Cancellable* cancellable) override;
    virtual void replace_on_next_command(const Cancellable* cancellable) override;


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


public:
    //  Superscalar Commands (the "ssf" framework)

    virtual void issue_barrier(const Cancellable* cancellable) override;
    virtual void issue_nop(const Cancellable* cancellable, Milliseconds duration) override;
    virtual void issue_buttons(
        const Cancellable* cancellable,
        Button button,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;
    virtual void issue_dpad(
        const Cancellable* cancellable,
        DpadPosition position,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;
    virtual void issue_left_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;
    virtual void issue_right_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;


public:
    //  High speed RPCs.

    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button, Milliseconds duration
    ) override;
    virtual void issue_mash_button(
        const Cancellable* cancellable,
        Button button0, Button button1, Milliseconds duration
    ) override;
    virtual void issue_mash_AZs(
        const Cancellable* cancellable,
        Milliseconds duration
    ) override;
    virtual void issue_system_scroll(
        const Cancellable* cancellable,
        DpadPosition direction, //  Diagonals not allowed.
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown
    ) override;


private:
    virtual void push_state(const Cancellable* cancellable, WallDuration duration) override;

    void update_status_string();

    class LoggingSuppressScope{
    public:
        LoggingSuppressScope(std::atomic<size_t>& counter)
            : m_counter(counter)
        {
            m_counter++;
        }
        ~LoggingSuppressScope(){
            m_counter--;
        }
    private:
        std::atomic<size_t>& m_counter;
    };


private:
    SerialLogger m_logger;
    BotBaseHandle m_handle;

    std::string m_status;
    std::string m_uptime;

    BotBaseController& m_serial;

    std::atomic<size_t> m_logging_suppress;
    SpinLock m_lock;
};




}
}
#endif
