/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <functional>
#include <deque>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
#include "ClientSource/Connection/BotBase.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Messages_PushButtons.h"
#include "NintendoSwitch_VirtualController.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



void KeyboardDebouncer::add_event(bool press, VirtualControllerState state){
    WallClock now = current_time();
    SpinLockGuard lg(m_lock);
    while (!m_history.empty()){
        if (m_history.front().timestamp + std::chrono::seconds(1) < now){
            m_history.pop_front();
        }else{
            break;
        }
    }
    m_history.emplace_back(Entry{now, press, state});
}

//  Returns when the consumer thread should wake up again.
WallClock KeyboardDebouncer::get_current_state(VirtualControllerState& state){
    WallClock now = current_time();

    const std::chrono::milliseconds RELEASE_DELAY(20);

    SpinLockGuard lg(m_lock);

    if (m_history.empty()){
        state = m_last;
        return WallClock::max();
    }

    auto iter = m_history.end();

    //  Iterate the history backwards until we find an event that can be used.
    //  Basically we use everything can be used except for button releases
    //  that are too new in order to debounce them.
    while (true){
        if (iter == m_history.begin()){
            break;
        }
        --iter;

        const Entry& entry = *iter;

        //  Last event is a press. Return it.
        if (entry.press){
            m_last = entry.state;
            ++iter;
            break;
        }

        //  Last event is a release that is old enough.
        if (entry.timestamp + RELEASE_DELAY < now){
            m_last = entry.state;
            ++iter;
            break;
        }

        //  Otherwise, skip this release event.
    }

    //  Clear the history that we don't need anymore.
    m_history.erase(m_history.begin(), iter);

    state = m_last;
    return m_history.empty()
        ? WallClock::max()
        : m_history.begin()->timestamp + RELEASE_DELAY;
}





VirtualController::VirtualController(
    LoggerQt& logger,
    BotBaseHandle& botbase,
    bool allow_commands_while_running
)
    : m_logger(logger)
    , m_botbase(botbase)
    , m_allow_commands_while_running(allow_commands_while_running)
    , m_last_known_state(ProgramState::STOPPED)
    , m_stop(false)
    , m_thread(run_with_catch, "VirtualController::thread_loop()", [=]{ thread_loop(); })
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
    m_controller_state = VirtualControllerState();
    m_pressed_buttons.clear();
}

void VirtualController::on_key_press(Qt::Key key){
    //  Suppress if key is already pressed.
    auto iter = m_pressed_buttons.find(key);
    if (iter != m_pressed_buttons.end()){
        return;
    }

    const ControllerButton* button = button_lookup(key);
    if (button == nullptr){
        return;
    }
    button->press(m_controller_state);

    m_pressed_buttons.insert(key);

    m_history.add_event(true, m_controller_state);

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}
void VirtualController::on_key_release(Qt::Key key){
    //  Suppress if key is not pressed.
    auto iter = m_pressed_buttons.find(key);
    if (iter == m_pressed_buttons.end()){
        return;
    }

    const ControllerButton* button = button_lookup(key);
    if (button == nullptr){
        return;
    }
    button->release(m_controller_state);

    m_pressed_buttons.erase(key);

    m_history.add_event(false, m_controller_state);

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}



void VirtualController::thread_loop(){
    VirtualControllerState last;
    bool last_neutral = true;
    WallClock last_press = current_time();
    while (true){
        if (m_stop.load(std::memory_order_acquire)){
            break;
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
        do{
            WallClock now = current_time();
            try{
//                current.print();
                if (current == last && last_press + std::chrono::milliseconds(1000) > now){
//                    cout << "No state change." << endl;
                    break;
                }

                //  Convert the state.
                ControllerState state;
                bool neutral = current.to_state(state);

                //  If state is neutral, just issue a stop.
                if (neutral){
                    m_botbase.try_stop_commands();
                    last = VirtualControllerState();
                    last_neutral = true;
                    last_press = now;
                    break;
                }

                //  If the new state is different, then set next interrupt so the new
                //  new command can replace the current one without gaps.
                if (!last_neutral && current != last && m_botbase.try_next_interrupt() != nullptr){
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
        if (next_wake == WallClock::max()){
            m_cv.wait_until(lg, next_wake);
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





