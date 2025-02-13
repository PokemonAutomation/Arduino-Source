/*  Nintendo Switch Pro Controller (Serial PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#ifndef PokemonAutomation_NintendoSwitch_ProController_SerialPABotBase_H
#define PokemonAutomation_NintendoSwitch_ProController_SerialPABotBase_H

#include "ClientSource/Connection/BotBase.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Connection.h"
#include "NintendoSwitch_ProControllerWithScheduler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class ProController_SerialPABotBase : public ProControllerWithScheduler{
public:
    using ContextType = ProControllerContext;

public:
    ProController_SerialPABotBase(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        const ControllerRequirements& requirements
    );

    virtual bool is_ready() const override{
        return m_serial && m_handle.is_ready() && m_error_string.empty();
    }
    virtual std::string error_string() const override{
        return m_error_string;
    }


public:
    //  General Control

    virtual void wait_for_all(const Cancellable* cancellable) override;
    virtual void cancel_all_commands(const Cancellable* cancellable) override;
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


private:
    SerialPABotBase::SerialPABotBase_Connection& m_handle;
    BotBaseController* m_serial;

    std::string m_error_string;
};




}
}
#endif
