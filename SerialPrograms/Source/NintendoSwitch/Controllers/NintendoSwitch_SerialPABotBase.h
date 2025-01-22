/*  Nintendo Switch Controller (Serial PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_NintendoSwitch_Controller_SerialPABotBase_H
#define PokemonAutomation_NintendoSwitch_Controller_SerialPABotBase_H

#include <QSerialPortInfo>
#include "ClientSource/Connection/BotBase.h"
#include "Controllers/ControllerDescriptor.h"
#include "Controllers/ControllerConnection.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Handle.h"
#include "NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class SwitchController_SerialPABotBase_Descriptor : public ControllerDescriptor{
public:
    static std::vector<std::unique_ptr<const ControllerDescriptor>> get_all_devices();

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




class SwitchController_SerialPABotBase :
    public ControllerConnection,
    public SwitchController
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
    BotBaseHandle& handle(){ return m_handle; } //  REMOVE


public:
    //  General Control

    virtual void wait_for_all(const Cancellable* cancellable) override;
    virtual void cancel_all(const Cancellable* cancellable) override;
    virtual void replace_on_next_command(const Cancellable* cancellable) override;


public:
    //  Basic Commands

    virtual void send_wait_for_pending(const Cancellable* cancellable) override;
    virtual void send_wait(const Cancellable* cancellable, uint16_t ticks) override;
    virtual void send_controller_state(
        const Cancellable* cancellable,
        Button button,
        DpadPosition position,
        uint8_t left_x, uint8_t left_y,
        uint8_t right_x, uint8_t right_y,
        uint16_t ticks
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

    virtual void send_buttons(
        const Cancellable* cancellable,
        Button button,
        uint16_t delay, uint16_t hold, uint8_t cooldown
    ) override;
    virtual void send_dpad(
        const Cancellable* cancellable,
        DpadPosition position,
        uint16_t delay, uint16_t hold, uint8_t cooldown
    ) override;
    virtual void send_left_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        uint16_t delay, uint16_t hold, uint8_t cooldown
    ) override;
    virtual void send_right_joystick(
        const Cancellable* cancellable,
        uint8_t x, uint8_t y,
        uint16_t delay, uint16_t hold, uint8_t cooldown
    ) override;


public:
    //  High speed RPCs.

    virtual void send_mash_button(
        const Cancellable* cancellable,
        Button button, uint16_t ticks
    ) override;
    virtual void send_mash_button(
        const Cancellable* cancellable,
        Button button0, Button button1, uint16_t ticks
    ) override;
    virtual void send_mash_AZs(
        const Cancellable* cancellable,
        uint16_t ticks
    ) override;
    virtual void send_system_scroll(
        const Cancellable* cancellable,
        DpadPosition direction, //  Diagonals not allowed.
        uint16_t delay, uint16_t hold, uint8_t cooldown
    ) override;


private:
    void update_status_string();


private:
    SerialLogger m_logger;
    BotBaseHandle m_handle;

    std::string m_status;
    std::string m_uptime;

    BotBaseController& m_serial;

};




}
}
#endif
