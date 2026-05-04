/*  PABotBase2: Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PABotBase2_Controller_H
#define PokemonAutomation_NintendoSwitch_PABotBase2_Controller_H

#include "Controllers/PABotBase2/PABotBase2_Connection.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerWithScheduler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class PABotBase2_Controller : public ControllerWithScheduler{
public:
    PABotBase2_Controller(
        Logger& logger,
        PABotBase2::Connection& connection
    )
        : ControllerWithScheduler(logger)
        , m_connection(connection)
    {}

    void stop_with_error(std::string error_message) noexcept;

    bool is_ready() const{
        return m_connection.is_ready();
    }
    std::string error_string() const;


public:
    void cancel_all_commands();
    void replace_on_next_command();

    void wait_for_all(Cancellable* cancellable);


protected:
    //  These are set on construction and never changed again. So it is safe to
    //  access these asynchronously.
    PABotBase2::Connection& m_connection;

    mutable SpinLock m_error_lock;
    std::string m_error_string;
};



}
}
#endif
