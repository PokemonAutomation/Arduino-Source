/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Messages_PushButtons.h"
#include "NintendoSwitch_VirtualController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


void KeyboardDebouncer::clear(){
    WriteSpinLock lg(m_lock);
    m_last = VirtualControllerState();
    m_history.clear();
}

void KeyboardDebouncer::add_event(bool press, VirtualControllerState state){
    WallClock now = current_time();
    WriteSpinLock lg(m_lock);
    while (!m_history.empty()){
        if (m_history.front().timestamp + std::chrono::seconds(1) < now){
            m_history.pop_front();
        }else{
            break;
        }
    }
    m_history.emplace_back(Entry{now, press, state});
//    cout << "add event" << endl;
}
WallClock KeyboardDebouncer::get_current_state(VirtualControllerState& state){
    WallClock now = current_time();

    const std::chrono::milliseconds RELEASE_DELAY(10);

    WriteSpinLock lg(m_lock);

    if (m_history.empty()){
//        cout << "empty" << endl;
        state = m_last;
        return WallClock::max();
    }

    auto iter = m_history.end();

    //  Iterate the history backwards until we find an event that can be used.
    //  Everything can be used except for key releases that are too new in
    //  order to debounce them.
    while (true){
        if (iter == m_history.begin()){
            break;
        }
        --iter;

        const Entry& entry = *iter;

        //  Key release that isn't old enough.
        if (entry.press || entry.timestamp + RELEASE_DELAY <= now){
            m_last = entry.state;
            ++iter;
            break;
        }
    }

    //  Clear the history that we don't need anymore.
    m_history.erase(m_history.begin(), iter);
//    cout << "remove event: " << m_history.size() << endl;

    state = m_last;
    return m_history.empty()
        ? WallClock::max()
        : m_history.begin()->timestamp + RELEASE_DELAY;
}





VirtualController::VirtualController(
    BotBaseHandle& botbase,
    bool allow_commands_while_running
)
    // : m_logger(logger)
    : m_botbase(botbase)
    , m_allow_commands_while_running(allow_commands_while_running)
    , m_last_known_state(ProgramState::STOPPED)
    , m_stop(false)
    , m_thread(run_with_catch, "VirtualController::thread_loop()", [this]{ thread_loop(); })
{}
VirtualController::~VirtualController(){
    m_stop.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lg(m_sleep_lock);
        m_cv.notify_all();
    }
    m_thread.join();
}
void VirtualController::clear_state(){
//    cout << "clear_state" << endl;
    m_controller_state = VirtualControllerState();
    m_pressed_buttons.clear();
    m_history.clear();

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}

bool VirtualController::on_key_press(Qt::Key key){
//    cout << "press: " << key << endl;

    const ControllerButton* button = button_lookup(key);
    if (button == nullptr){
        return false;
    }

    //  Suppress if key is already pressed.
    auto iter = m_pressed_buttons.find(button);
    if (iter != m_pressed_buttons.end()){
        return true;
    }
    button->press(m_controller_state);

    m_pressed_buttons.insert(button);
    m_history.add_event(true, m_controller_state);

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
    return true;
}
bool VirtualController::on_key_release(Qt::Key key){
//    cout << "release" << endl;

    const ControllerButton* button = button_lookup(key);
    if (button == nullptr){
        return false;
    }

    //  Suppress if key is not pressed.
    auto iter = m_pressed_buttons.find(button);
    if (iter == m_pressed_buttons.end()){
        return true;
    }
    button->release(m_controller_state);

    m_pressed_buttons.erase(button);
    m_history.add_event(false, m_controller_state);

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
    return true;
}



void VirtualController::thread_loop(){
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread();

    VirtualControllerState last;
    bool last_neutral = true;
    WallClock last_press = current_time();
    while (true){
        if (m_stop.load(std::memory_order_acquire)){
            return;
        }

        //  Not accepting commands.
        if (!m_allow_commands_while_running &&
            m_last_known_state.load(std::memory_order_acquire) != ProgramState::STOPPED
        ){
            last = VirtualControllerState();
            std::unique_lock<std::mutex> lg(m_sleep_lock);
            if (m_stop.load(std::memory_order_acquire)){
                return;
            }
            m_cv.wait(lg);
            continue;
        }

        VirtualControllerState current;
        WallClock next_wake = m_history.get_current_state(current);
//        current.print();

        //  Send the command.
        WallClock now;
        do{
            now = current_time();
            try{
//                current.print();
                if (current == last && last_press + std::chrono::milliseconds(1000) > now){
//                    cout << "No state change." << endl;
                    break;
                }

                //  Convert the state.
                ControllerState state;
                bool neutral = current.to_state(state);
//                cout << "neutral = " << neutral << endl;

                //  If state is neutral, just issue a stop.
                if (neutral){
                    if (m_botbase.try_stop_commands() == nullptr){
                        last = VirtualControllerState();
                        last_neutral = true;
                        last_press = now;
                    }else{
                        next_wake = now + std::chrono::milliseconds(PABB_RETRANSMIT_DELAY_MILLIS);
                    }
                    break;
                }

                //  If the new state is different, set next interrupt so the new
                //  new command can replace the current one without gaps.
                if (!last_neutral && current != last && m_botbase.try_next_interrupt() != nullptr){
                    next_wake = now + std::chrono::milliseconds(PABB_RETRANSMIT_DELAY_MILLIS);
                    break;
                }

                //  Send the command.
                DeviceRequest_controller_state request(
                    state.buttons,
                    state.dpad,
                    state.left_x,
                    state.left_y,
                    state.right_x,
                    state.right_y,
                    255
                );
                if (m_botbase.try_send_request(request) != nullptr){
                    next_wake = now + std::chrono::milliseconds(PABB_RETRANSMIT_DELAY_MILLIS);
                    break;
                }

                last = current;
                last_neutral = false;
                last_press = now;
            }catch (ProgramCancelledException&){
            }catch (ProgramFinishedException&){
            }catch (InvalidConnectionStateException&){
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


ProgramState VirtualController::last_known_state() const{
    return m_last_known_state.load(std::memory_order_acquire);
}
void VirtualController::on_state_changed(ProgramState state){
    m_last_known_state.store(state, std::memory_order_release);
}




}
}





