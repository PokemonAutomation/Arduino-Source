/*  Keyboard Input
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_KeyboardInput_H
#define PokemonAutomation_Controllers_KeyboardInput_H

#include <thread>
#include <condition_variable>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "KeyboardStateTracker.h"

class QKeyEvent;

namespace PokemonAutomation{


class ControllerSession;




class ControllerState{
public:
    virtual ~ControllerState() = default;

    virtual void clear() = 0;

    virtual bool operator==(const ControllerState& x) const = 0;
    virtual bool operator!=(const ControllerState& x) const{ return !(*this == x); }

    virtual bool is_neutral() const = 0;
};




class KeyboardInputController{
public:
    KeyboardInputController(bool enabled);
    virtual ~KeyboardInputController();


public:
    void clear_state();

    void on_key_press(const QKeyEvent& key);
    void on_key_release(const QKeyEvent& key);


protected:
    void start();           //  Child class must call this at end of constructor.
    void stop() noexcept;   //  Child class must call this at start of destructor.

    virtual std::unique_ptr<ControllerState> make_state() const = 0;
    virtual void update_state(ControllerState& state, const std::set<uint32_t>& pressed_keys) = 0;
    virtual void cancel_all_commands() = 0;
    virtual void replace_on_next_command() = 0;
    virtual void send_state(const ControllerState& state) = 0;

private:
    void thread_loop();


private:
    //  Controller State
    SpinLock m_state_lock;
    KeyboardStateTracker m_state_tracker;

    std::atomic<bool> m_stop;

    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};





}
#endif
