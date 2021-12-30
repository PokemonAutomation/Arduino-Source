/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <functional>
#include <deque>
#include "Common/Cpp/Exception.h"
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
#include "ClientSource/Connection/BotBase.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "VirtualSwitchController.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


VirtualController::VirtualController(BotBaseHandle& botbase, Logger& logger)
    : m_botbase(botbase)
    , m_logger(logger)
    , m_last(std::chrono::system_clock::now())
    , m_last_known_state(ProgramState::STOPPED)
    , m_stop(false)
    , m_granularity(1)
    , m_thread(&VirtualController::thread_loop, this)
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
    {
        SpinLockGuard lg(m_state_lock, "VirtualController::on_key_press()");

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
        m_last = std::chrono::system_clock::now();
    }
//    print();
    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}
void VirtualController::on_key_release(Qt::Key key){
    {
        SpinLockGuard lg(m_state_lock, "VirtualController::on_key_release()");

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
        m_last = std::chrono::system_clock::now();
    }
//    print();

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}



void VirtualController::thread_loop(){

#if 1
    std::deque<std::chrono::milliseconds> delays;
    std::chrono::milliseconds sum(0);
//    bool spin_mode = false;
#endif

    while (true){
        if (m_stop.load(std::memory_order_acquire)){
            break;
        }

        Button buttons;
        DpadPosition dpad;
        uint8_t left_x;
        uint8_t left_y;
        uint8_t right_x;
        uint8_t right_y;
        bool neutral;
        {
            SpinLockGuard lg(m_state_lock, "VirtualController::thread_loop()");

            buttons = m_controller_state.buttons;
            neutral = m_controller_state.buttons == 0;

            dpad = DPAD_NONE;
            if (m_controller_state.dpad_x != 0 || m_controller_state.dpad_y != 0){
                neutral = false;
                do{
                    if (m_controller_state.dpad_x == 0){
                        dpad = m_controller_state.dpad_y > 0 ? DPAD_DOWN : DPAD_UP;
                        break;
                    }
                    if (m_controller_state.dpad_y == 0){
                        dpad = m_controller_state.dpad_x > 0 ? DPAD_RIGHT : DPAD_LEFT;
                        break;
                    }
                    if (m_controller_state.dpad_x < 0 && m_controller_state.dpad_y < 0){
                        dpad = DPAD_UP_LEFT;
                        break;
                    }
                    if (m_controller_state.dpad_x < 0 && m_controller_state.dpad_y > 0){
                        dpad = DPAD_DOWN_LEFT;
                        break;
                    }
                    if (m_controller_state.dpad_x > 0 && m_controller_state.dpad_y > 0){
                        dpad = DPAD_DOWN_RIGHT;
                        break;
                    }
                    if (m_controller_state.dpad_x > 0 && m_controller_state.dpad_y < 0){
                        dpad = DPAD_UP_RIGHT;
                        break;
                    }
                }while (false);
            }

            left_x = 128;
            left_y = 128;
            if (m_controller_state.left_joystick_x != 0 || m_controller_state.left_joystick_y != 0){
                neutral = false;
                int mag = std::abs(m_controller_state.left_joystick_x) > std::abs(m_controller_state.left_joystick_y)
                    ? std::abs(m_controller_state.left_joystick_x)
                    : std::abs(m_controller_state.left_joystick_y);
                left_x = std::min(128 * m_controller_state.left_joystick_x / mag + 128, 255);
                left_y = std::min(128 * m_controller_state.left_joystick_y / mag + 128, 255);
            }

            right_x = 128;
            right_y = 128;
            if (m_controller_state.right_joystick_x != 0 || m_controller_state.right_joystick_y != 0){
                neutral = false;
                int mag = std::abs(m_controller_state.right_joystick_x) > std::abs(m_controller_state.right_joystick_y)
                    ? std::abs(m_controller_state.right_joystick_x)
                    : std::abs(m_controller_state.right_joystick_y);
                right_x = std::min(128 * m_controller_state.right_joystick_x / mag + 128, 255);
                right_y = std::min(128 * m_controller_state.right_joystick_y / mag + 128, 255);
            }

        }

        if (neutral){
            std::unique_lock<std::mutex> lg(m_sleep_lock);
            if (m_stop.load(std::memory_order_acquire)){
                return;
            }
            m_cv.wait(lg);
            continue;
        }

        do{
//            cout << (int)m_last_known_state.load(std::memory_order_acquire) << endl;
            if (m_last_known_state.load(std::memory_order_acquire) != ProgramState::STOPPED){
                break;
            }
            try{
                DeviceRequest_controller_state request(
                    buttons, dpad,
                    left_x, left_y,
                    right_x, right_y,
                    m_granularity
                );
                m_botbase.try_send_request(request);
            }catch (PokemonAutomation::CancelledException&){
            }catch (const StringException&){
            }
        }while (false);




#if 0
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        if (m_stop.load(std::memory_order_acquire)){
            break;
        }
        m_cv.wait_for(lg, std::chrono::milliseconds(4));
#else
//        if (spin_mode){
//            _mm_pause();
//            continue;
//        }

        //
        //  Adaptive Granularity:
        //
        //      On some computers, the sleep/yield latency is very high.
        //  For example, Sleep(1) or cv.wait_for(..., 1ms) doesn't return in 1ms,
        //  but maybe as high as 16 or 32ms. This is so large that it causes us
        //  to drop frames.
        //
        //  To make this actually work, we measure the sleep latency and adjust
        //  the granularity accordingly. Longer latencies will require larger
        //  granularity to avoid dropping frames. But larger granularity also
        //  decreases keyboard->Switch responsiveness.
        //

        const uint16_t TICK_MILLIS = 1000 / TICKS_PER_SECOND;
        std::chrono::milliseconds granularity = std::chrono::milliseconds(TICK_MILLIS * m_granularity);

        auto start = std::chrono::system_clock::now();
        {
            std::unique_lock<std::mutex> lg(m_sleep_lock);
            if (m_stop.load(std::memory_order_acquire)){
                break;
            }
            m_cv.wait_for(lg, granularity / 2);
        }
        auto stop = std::chrono::system_clock::now();
        std::chrono::milliseconds delay = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        if (delays.size() > 20){
            sum -= delays[0];
            delays.pop_front();

        }

        delays.push_back(delay);
        sum += delay;

        auto average = sum / delays.size();
        if (delays.size() > 10 && average > granularity){
//            spin_mode = true;
            if (m_granularity < 4){
                m_granularity++;
                m_logger.log(
                    "System sleep latency (" + QString::number(average.count()) + " ms). "
                    "Increasing granularity to " + QString::number(m_granularity) + " ticks.",
                    COLOR_RED
                );
            }
        }else if (delays.size() > 10 && average < granularity / 2){
            if (m_granularity > 1){
                m_granularity--;
                m_logger.log(
                    "System sleep latency (" + QString::number(average.count()) + " ms). "
                    "Decreasing granularity to " + QString::number(m_granularity) + " ticks.",
                    COLOR_BLUE
                );
            }
        }
#endif
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





