/*  SerialPABotBase: Wireless Pro Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessProController_H
#define PokemonAutomation_NintendoSwitch_SerialPABotBase_WirelessProController_H

#include "NintendoSwitch_SerialPABotBase_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SerialPABotBase_WirelessProController final : public SerialPABotBase_ProController{
public:
    using ContextType = ProControllerContext;

public:
    SerialPABotBase_WirelessProController(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        const ControllerRequirements& requirements
    );
    ~SerialPABotBase_WirelessProController();
    void stop();

public:
    virtual ControllerType controller_type() const override{
        return ControllerType::NintendoSwitch_WirelessProController;
    }
    virtual Milliseconds ticksize() const override{
        return Milliseconds(15);
    }
    virtual Milliseconds timing_variation() const override{
        return Milliseconds::zero();
    }

public:
    class MessageControllerStatus;
    class MessageControllerState;

private:
    template <typename Type>
    PA_FORCE_INLINE Type milliseconds_to_ticks_15ms(Type milliseconds){
        return milliseconds / 15 + (milliseconds % 15 + 14) / 15;
    }
    virtual void push_state(const Cancellable* cancellable, WallDuration duration) override;

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
