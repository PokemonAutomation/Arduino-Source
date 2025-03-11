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

    bool is_ready() const{
        return m_serial && m_handle.is_ready();
    }


public:
    void cancel_all_commands();
    void replace_on_next_command();

    void wait_for_all(const Cancellable* cancellable);

    void send_botbase_request(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    );
    BotBaseMessage send_botbase_request_and_wait(
        const Cancellable* cancellable,
        const BotBaseRequest& request
    );


protected:
    //  These are set on construction and never changed again. So it is safe to
    //  access these asynchronously.
    SerialPABotBase::SerialPABotBase_Connection& m_handle;
    BotBaseController* m_serial;
    ControllerFeatures m_supported_features;
};




}
}
#endif
