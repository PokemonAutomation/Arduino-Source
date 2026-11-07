/*  Global Gamepad Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_Gamepad_GlobalGamepadTracker_H
#define PokemonAutomation_ControllerInput_Gamepad_GlobalGamepadTracker_H

#ifdef PA_SDL

#include <atomic>
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "ControllerInput/ControllerInput.h"

namespace PokemonAutomation{

class GamepadTracker final : public ControllerInputSource{
public:
    ~GamepadTracker();

    // Initialize and shut down SDL on the main thread, while QApplication lives.
    void start();
    virtual void stop() noexcept override;
    virtual void clear_state() override;

private:
    void thread_loop();

private:
    bool m_initialized{false};
    std::atomic<bool> m_stopping{false};
    std::atomic<bool> m_reset{true};
    Mutex m_sleep_lock;
    ConditionVariable m_cv;
    AsyncTask m_thread;
};

GamepadTracker& global_gamepad_tracker();

}
#endif
#endif
