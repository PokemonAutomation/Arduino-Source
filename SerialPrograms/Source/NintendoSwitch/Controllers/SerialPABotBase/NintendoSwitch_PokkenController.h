/*  Pokken Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PokkenController_H
#define PokemonAutomation_NintendoSwitch_PokkenController_H

#include "Controllers/ControllerCapability.h"
#include "NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SerialPABotBase_PokkenController final : public SerialPABotBase_ProController{
public:
    using ContextType = ProControllerContext;

public:
    SerialPABotBase_PokkenController(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        const ControllerRequirements& requirements
    );
    ~SerialPABotBase_PokkenController();

public:
    virtual ControllerType controller_type() const override{
        return ControllerType::NintendoSwitch_WiredProController;
    }
    virtual Milliseconds ticksize() const override{
        return Milliseconds(8);
    }
    virtual Milliseconds timing_variation() const override{
        return Milliseconds::zero();
    }

private:
    template <typename Type>
    PA_FORCE_INLINE Type milliseconds_to_ticks_8ms(Type milliseconds){
        return milliseconds / 8 + (milliseconds % 8 + 7) / 8;
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
