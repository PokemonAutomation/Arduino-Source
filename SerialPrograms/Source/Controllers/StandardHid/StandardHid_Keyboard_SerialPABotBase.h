/*  SerialPABotBase: HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StandardHid_Keyboard_SerialPABotBase_H
#define PokemonAutomation_StandardHid_Keyboard_SerialPABotBase_H

#include "ClientSource/Connection/BotBase.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Connection.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_StatusThread.h"
#include "StandardHid_Keyboard.h"
#include "StandardHid_KeyboardWithScheduler.h"

namespace PokemonAutomation{
namespace StandardHid{



class SerialPABotBase_Keyboard final :
    public Keyboard,
    public KeyboardControllerWithScheduler,
    private SerialPABotBase::ControllerStatusThreadCallback
{
public:
    using ContextType = KeyboardContext;


public:
    SerialPABotBase_Keyboard(
        Logger& logger,
        SerialPABotBase::SerialPABotBase_Connection& connection,
        ControllerResetMode reset_mode
    );
    ~SerialPABotBase_Keyboard();
    void stop();

    virtual Logger& logger() override{
        return m_logger;
    }
    virtual RecursiveThrottler& logging_throttler() override{
        return m_logging_throttler;
    }
    virtual bool is_ready() const override{
        return m_serial
            && m_serial->state() == BotBaseController::State::RUNNING
            && m_handle.is_ready();
    }


public:
    virtual ControllerType controller_type() const override{
        return ControllerType::HID_Keyboard;
    }
    virtual ControllerClass controller_class() const override{
        return ControllerClass::KEYBOARD;
    }
    virtual ControllerPerformanceClass performance_class() const override{
        return ControllerPerformanceClass::SerialPABotBase_Wired;
    }

    virtual Milliseconds ticksize() const override{
        return Milliseconds(0);
    }
    virtual Milliseconds cooldown() const override{
        return Milliseconds(8);
    }
    virtual Milliseconds timing_variation() const override{
        return Milliseconds(0);
    }
    virtual bool atomic_multibutton() const override{
        return true;
    }


public:
    //  Superscalar Commands (the "ssf" framework)

    virtual void issue_barrier(const Cancellable* cancellable) override{
        KeyboardControllerWithScheduler::issue_barrier(cancellable);
    }
    virtual void issue_nop(const Cancellable* cancellable, Milliseconds duration) override{
        KeyboardControllerWithScheduler::issue_nop(cancellable, duration);
    }
    virtual void issue_key(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        KeyboardKey key
    ) override{
        KeyboardControllerWithScheduler::issue_key(cancellable, delay, hold, cooldown, key);
    }
    virtual void issue_keys(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const std::vector<KeyboardKey>& keys
    ) override{
        KeyboardControllerWithScheduler::issue_keys(cancellable, delay, hold, cooldown, keys);
    }


public:
    virtual void update_status(Cancellable& cancellable) override;
    virtual void stop_with_error(std::string error_message) override;

    std::string error_string() const{
        ReadSpinLock lg(m_error_lock);
        return m_error_string;
    }


public:
    virtual void cancel_all_commands() override;
    virtual void replace_on_next_command() override;

    virtual void wait_for_all(const Cancellable* cancellable) override;


private:
    virtual void execute_state(
        const Cancellable* cancellable,
        const SuperscalarScheduler::ScheduleEntry& entry
    ) override;

    void status_thread();


protected:
    //  These are set on construction and never changed again. So it is safe to
    //  access these asynchronously.
    SerialPABotBase::SerialPABotBase_Connection& m_handle;
    BotBaseController* m_serial;

    mutable SpinLock m_error_lock;
    std::string m_error_string;

    std::map<uint64_t, KeyboardKey> m_last_state;

    std::unique_ptr<SerialPABotBase::ControllerStatusThread> m_status_thread;
};






}
}
#endif
