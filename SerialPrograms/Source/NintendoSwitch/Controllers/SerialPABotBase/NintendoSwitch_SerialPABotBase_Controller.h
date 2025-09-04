/*  SerialPABotBase: Pro Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_Controller_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_Controller_H

#include "ClientSource/Connection/BotBase.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Connection.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerWithScheduler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SerialPABotBase_Controller : public ControllerWithScheduler{
public:
    SerialPABotBase_Controller(
        Logger& logger,
        ControllerType controller_type,
        SerialPABotBase::SerialPABotBase_Connection& connection
    );

    void stop_with_error(std::string error_message){
        {
            WriteSpinLock lg(m_error_lock);
            m_error_string = error_message;
        }
        m_serial->stop(std::move(error_message));
    }

    bool is_ready() const{
        return m_serial
            && m_serial->state() == BotBaseController::State::RUNNING
            && m_handle.is_ready();
    }
    std::string error_string() const{
        ReadSpinLock lg(m_error_lock);
        return m_error_string;
    }


public:
    void cancel_all_commands();
    void replace_on_next_command();

    void wait_for_all(const Cancellable* cancellable);


protected:
    //  These are set on construction and never changed again. So it is safe to
    //  access these asynchronously.
    SerialPABotBase::SerialPABotBase_Connection& m_handle;
    BotBaseController* m_serial;

    mutable SpinLock m_error_lock;
    std::string m_error_string;
};




}
}
#endif
