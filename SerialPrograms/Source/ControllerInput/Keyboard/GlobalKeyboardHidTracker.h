/*  Global Keyboard HID Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_Keyboard_GlobalKeyboardHidTracker_H
#define PokemonAutomation_ControllerInput_Keyboard_GlobalKeyboardHidTracker_H

#include <set>
#include "Common/Cpp/Logging/TaggedLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "ControllerInput/ControllerInput.h"
//#include "KeyboardHidButtons.h"
#include "KeyboardInput_State.h"
#include "KeyboardInput_StateTracker.h"

class QKeyEvent;

namespace PokemonAutomation{


class KeyboardHidTracker final : public ControllerInputSource{
public:
    ~KeyboardHidTracker();
    KeyboardHidTracker();

    virtual void stop() noexcept override;

    virtual void clear_state() override;

    void on_key_press(const QKeyEvent& key);
    void on_key_release(const QKeyEvent& key);


private:
    KeyboardInputState keys_to_state(const std::set<uint32_t>& pressed_native_keys) const;
    void thread_loop();


private:
    TaggedLogger m_logger;

    //  Controller State
    SpinLock m_state_lock;
    KeyboardStateTracker m_state_tracker;

    std::atomic<bool> m_stopping;

    Mutex m_sleep_lock;
    ConditionVariable m_cv;
    AsyncTask m_thread;
};


KeyboardHidTracker& global_keyboard_tracker();




}
#endif
