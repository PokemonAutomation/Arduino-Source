/*  Keyboard Input
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "Controllers/KeyboardInput/GlobalQtKeyMap.h"
#include "KeyboardInput.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



KeyboardInputController::~KeyboardInputController() = default;
KeyboardInputController::KeyboardInputController(Logger& logger, bool enabled)
    : m_logger(logger)
    , m_stop(false)
{}

void KeyboardInputController::start(){
    m_thread = std::thread(
        run_with_catch,
        "KeyboardInputController::thread_loop()",
        [this]{ thread_loop(); }
    );
}
void KeyboardInputController::stop() noexcept{
    bool expected = false;
    if (!m_stop.compare_exchange_strong(expected, true)){
        //  Already stopped.
        return;
    }
    {
        std::lock_guard<std::mutex> lg(m_sleep_lock);
        m_cv.notify_all();
    }
    if (m_thread.joinable()){
        m_thread.join();
    }
}

void KeyboardInputController::clear_state(){
    {
        WriteSpinLock lg(m_state_lock);
        m_state_tracker.clear();
    }

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}
void KeyboardInputController::on_key_press(const QKeyEvent& key){
//    cout << "press: " << key.key() << ", native = " << key.nativeVirtualKey() << ", scancode = " << key.nativeScanCode() << endl;

//    QKeySequence seq((Qt::Key)key.key());
//    cout << "button: " << seq.toString().toStdString() << endl;

    QtKeyMap::instance().record(key);
    {
        WriteSpinLock lg(m_state_lock);
        m_state_tracker.press(key.nativeVirtualKey());
    }

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}
void KeyboardInputController::on_key_release(const QKeyEvent& key){
//    cout << "release: " << key.key() << ", native = " << key.nativeVirtualKey() << endl;
    QtKeyMap::instance().record(key);
    {
        WriteSpinLock lg(m_state_lock);
        m_state_tracker.release(key.nativeVirtualKey());
    }

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}



void KeyboardInputController::thread_loop(){
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread(m_logger);

    std::unique_ptr<ControllerState> last = make_state();
    std::unique_ptr<ControllerState> current = make_state();

    bool last_neutral = true;
    WallClock last_press = current_time();
    while (true){
        if (m_stop.load(std::memory_order_acquire)){
            return;
        }

#if 0
        //  Not accepting commands.
        if (!m_enabled){
            last->clear();
            std::unique_lock<std::mutex> lg(m_sleep_lock);
            if (m_stop.load(std::memory_order_acquire)){
                return;
            }
            m_cv.wait(lg);
            continue;
        }
#endif

        //  Get the raw keyboard state.
        std::set<uint32_t> pressed_native_keys;
        WallClock next_wake;
        {
            ReadSpinLock lg(m_state_lock);
            pressed_native_keys = m_state_tracker.get_currently_pressed();
            next_wake = m_state_tracker.next_state_change();
        }

        update_state(*current, pressed_native_keys);
        bool neutral = current->is_neutral();


        //  Send the command.
        WallClock now;
        do{
            now = current_time();
            if (neutral && last_neutral){
                continue;
            }
            try{
//                current.print();
                if (*current == *last && last_press + std::chrono::milliseconds(1000) > now){
//                    cout << "No state change." << endl;
                    break;
                }

                //  If state is neutral, just issue a stop.
                if (neutral){
                    cancel_all_commands();
                    last->clear();
                    last_neutral = true;
                    last_press = now;
                    break;
                }

                //  If the new state is different, set next interrupt so the new
                //  new command can replace the current one without gaps.
                if (!last_neutral && current != last){
                    replace_on_next_command();
                }

                //  Send the command.
                send_state(*current);

                std::swap(last, current);
                last_neutral = false;
                last_press = now;
            }catch (Exception& e){
                e.log(global_logger_tagged());
            }
        }while (false);


        //  Wait for next event.
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        if (m_stop.load(std::memory_order_acquire)){
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
