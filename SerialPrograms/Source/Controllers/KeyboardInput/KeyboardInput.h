/*  Keyboard Input
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_KeyboardInput_H
#define PokemonAutomation_Controllers_KeyboardInput_H

#include <thread>
#include <condition_variable>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Globals.h"
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
    virtual bool send_to_controller(ControllerSession& controller) const = 0;
};




class KeyboardInputController{
public:
    KeyboardInputController(
        bool allow_commands_while_running,
        std::chrono::milliseconds retry_delay = std::chrono::milliseconds(10)
    );
    virtual ~KeyboardInputController();


public:
    void clear_state();

    void on_key_press(const QKeyEvent& key);
    void on_key_release(const QKeyEvent& key);


public:
    //  Call from any thread.
    ProgramState last_known_state() const;
    void on_state_changed(ProgramState state);


protected:
    void start();   //  Child class must call this at end of constructor.
    void stop();    //  Child class must call this at start of destructor.

    virtual std::unique_ptr<ControllerState> make_state() const = 0;
    virtual void update_state(ControllerState& state, const std::set<uint32_t>& pressed_keys) = 0;
    virtual bool try_stop_commands() = 0;
    virtual bool try_next_interrupt() = 0;
    virtual bool try_send_state(const ControllerState& state) = 0;

private:
    void thread_loop();


private:
    const bool m_allow_commands_while_running;
    const std::chrono::milliseconds m_retry_delay;

    //  Controller State
    SpinLock m_state_lock;
    KeyboardStateTracker m_state_tracker;

    std::atomic<ProgramState> m_last_known_state;
    std::atomic<bool> m_stop;

    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};





}
#endif
