/*  Global Keyboard HID Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_Keyboard_GlobalKeyboardHidTracker_H
#define PokemonAutomation_ControllerInput_Keyboard_GlobalKeyboardHidTracker_H

#include <set>
#include <condition_variable>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "CommonFramework/Logging/Logger.h"
#include "ControllerInput/ControllerInput.h"
//#include "KeyboardHidButtons.h"
#include "KeyboardInput_State.h"
#include "KeyboardInput_StateTracker.h"

class QKeyEvent;

namespace PokemonAutomation{


class KeyboardHidTracker{
public:
    void add_listener(ControllerInputListener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(ControllerInputListener& listener){
        m_listeners.remove(listener);
    }


public:
    ~KeyboardHidTracker();
    KeyboardHidTracker();

    void stop();

    void clear_state();

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

    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    Thread m_thread;

    ListenerSet<ControllerInputListener> m_listeners;
};


KeyboardHidTracker& global_keyboard_tracker();




}
#endif
