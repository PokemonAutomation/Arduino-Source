/*  SerialPABotBase: Wireless Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessController_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessController_H

#include "Common/NintendoSwitch/NintendoSwitch_Protocol_ESP32.h"
#include "NintendoSwitch_SerialPABotBase_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SerialPABotBase_WirelessController : public SerialPABotBase_Controller{
public:
    SerialPABotBase_WirelessController(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        ControllerType controller_type,
        const ControllerRequirements& requirements
    );
    ~SerialPABotBase_WirelessController();
    void stop();


public:
    Milliseconds ticksize() const{
        return Milliseconds(15);
    }
    Milliseconds timing_variation() const{
        return Milliseconds::zero();
    }


public:
    class MessageControllerStatus;
    class MessageControllerState;


protected:
    void issue_report(
        const Cancellable* cancellable,
        const ESP32Report0x30& report,
        WallDuration duration
    );

private:
    template <typename Type>
    PA_FORCE_INLINE Type milliseconds_to_ticks_15ms(Type milliseconds){
        return milliseconds / 15 + (milliseconds % 15 + 14) / 15;
    }

    void status_thread();


private:
    CancellableHolder<CancellableScope> m_scope;
    std::atomic<bool> m_stopping;
    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    std::thread m_status_thread;
};



}
}
#endif
