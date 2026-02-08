/*  Global Keyboard HID Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "KeyboardInput_State.h"
#include "KeyboardInput_KeyMappings.h"
#include "GlobalKeyboardHidTracker.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




KeyboardHidTracker& global_keyboard_tracker(){
    static KeyboardHidTracker tracker;
    return tracker;
}




KeyboardHidTracker::~KeyboardHidTracker(){
    stop();
}
KeyboardHidTracker::KeyboardHidTracker()
    : m_logger(global_logger_raw(), "Keyboard")
    , m_stopping(false)
    , m_thread(
        GlobalThreadPools::unlimited_realtime().blocking_dispatch([this]{
            run_with_catch(
                "KeyboardHidTracker::thread_loop()",
                [this]{ thread_loop(); }
            );
        })
    )
{}


void KeyboardHidTracker::stop() noexcept{
    if (!m_thread){
        return;
    }
    m_stopping.store(true, std::memory_order_release);
    {
        std::lock_guard<Mutex> lg(m_sleep_lock);
    }
    m_cv.notify_all();
    m_thread.wait_and_ignore_exceptions();
}


void KeyboardHidTracker::clear_state(){
    {
        WriteSpinLock lg(m_state_lock);
        m_state_tracker.clear();
    }

    std::lock_guard<Mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}

void KeyboardHidTracker::on_key_press(const QKeyEvent& key){
    QtKeyMap::instance().record(key);
    {
        WriteSpinLock lg(m_state_lock);
        m_state_tracker.press(key.nativeVirtualKey());
    }

    std::lock_guard<Mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}
void KeyboardHidTracker::on_key_release(const QKeyEvent& key){
    QtKeyMap::instance().record(key);
    {
        WriteSpinLock lg(m_state_lock);
        m_state_tracker.release(key.nativeVirtualKey());
    }

    std::lock_guard<Mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}


KeyboardInputState KeyboardHidTracker::keys_to_state(const std::set<uint32_t>& pressed_native_keys) const{
    const QtKeyMap& qkey_map = QtKeyMap::instance();
    const KeyboardInputMappings& hid_map = get_keyid_to_hid_map();

    KeyboardInputState ret;
    for (uint32_t native_key : pressed_native_keys){
        std::set<QtKeyMap::QtKey> qkeys = qkey_map.get_QtKeys(native_key);
        for (const QtKeyMap::QtKey& qkey : qkeys){
//            cout << "qkey = " << qkey.key << " : " << qkey.keypad << endl;
            KeyboardKey key = hid_map.get(qkey);
            if (key != KeyboardKey::KEY_NONE){
                ret.add(key);
            }
//            log_qtkey(m_logger, qkey);
        }
    }

    return ret;
}


void KeyboardHidTracker::thread_loop(){
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread(m_logger);

    KeyboardInputState last;
    KeyboardInputState current;

    bool last_neutral = true;
    WallClock last_press = current_time();
    while (!m_stopping.load(std::memory_order_acquire)){
        //  Get the raw keyboard state.
        std::set<uint32_t> pressed_native_keys;
        WallClock next_wake;
        {
            ReadSpinLock lg(m_state_lock);
            pressed_native_keys = m_state_tracker.get_currently_pressed();
            next_wake = m_state_tracker.next_state_change();
        }

        current = keys_to_state(pressed_native_keys);

#if 0
        std::string str;
        for (KeyboardKey key : current.keys()){
            if (!str.empty()){
                str += ", ";
            }
            auto iter = KEYBOARDKEY_TO_STRING().find(key);
            if (iter != KEYBOARDKEY_TO_STRING().end()){
                str += iter->second;
            }
        }
        if (!str.empty()){
            cout << "Key State: " << str << endl;
        }
#endif

        bool neutral = current.is_neutral();
//        cout << "neutral = " << neutral << endl;

        //  Send the command.
        WallClock now;
        do{
            now = current_time();
            if (neutral && last_neutral){
                continue;
            }
            try{
//                current.print();
                if (current == last && last_press + std::chrono::milliseconds(1000) > now){
//                    cout << "No state change." << endl;
                    break;
                }

                //  If state is neutral, just issue a stop.
                if (neutral){
                    m_listeners.run_method(&ControllerInputListener::run_controller_input, current);
//                    cancel_all_commands();
                    last.clear();
                    last_neutral = true;
                    last_press = now;
                    break;
                }

                //  If the new state is different, set next interrupt so the new
                //  new command can replace the current one without gaps.
                if (!last_neutral && current != last){
//                    replace_on_next_command();
                }

                //  Send the command.
//                cout << "send_state()" << endl;
                m_listeners.run_method(&ControllerInputListener::run_controller_input, current);
//                send_state(*current);

                std::swap(last, current);
                last_neutral = false;
                last_press = now;
            }catch (Exception& e){
                e.log(global_logger_tagged());
            }
        }while (false);


        //  Wait for next event.
        std::unique_lock<Mutex> lg(m_sleep_lock);
        if (m_stopping.load(std::memory_order_acquire)){
            return;
        }

        //  Nothing held down.
        if (last_neutral && next_wake == WallClock::max()){
//            cout << "wait - long" << endl;
            m_cv.wait(lg);
        }else{
//            cout << "wait - short" << endl;
            m_cv.wait_until(lg, std::min(next_wake, now + std::chrono::milliseconds(1000)));
        }
    }
}






}
