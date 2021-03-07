/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSwShMisc.h"
#include "ClientSource/Connection/BotBase.h"
#include "VirtualSwitchController.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


VirtualController::VirtualController(BotBaseHandle& botbase, Logger& logger)
    : m_botbase(botbase)
    , m_logger(logger)
    , m_buttons(0)
    , m_dpad_x(0)
    , m_dpad_y(0)
    , m_left_joystick_x(0)
    , m_left_joystick_y(0)
    , m_right_joystick_x(0)
    , m_right_joystick_y(0)
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
    m_buttons = 0;
    m_dpad_x = 0;
    m_dpad_y = 0;
    m_left_joystick_x = 0;
    m_left_joystick_y = 0;
    m_right_joystick_x = 0;
    m_right_joystick_y = 0;
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

        switch (key){
        case Qt::Key::Key_8:
            m_dpad_y--;
            break;
        case Qt::Key::Key_9:
            m_dpad_x++;
            m_dpad_y--;
            break;
        case Qt::Key::Key_6:
            m_dpad_x++;
            break;
        case Qt::Key::Key_3:
            m_dpad_x++;
            m_dpad_y++;
            break;
        case Qt::Key::Key_2:
            m_dpad_y++;
            break;
        case Qt::Key::Key_1:
            m_dpad_x--;
            m_dpad_y++;
            break;
        case Qt::Key::Key_4:
            m_dpad_x--;
            break;
        case Qt::Key::Key_7:
            m_dpad_x--;
            m_dpad_y--;
            break;

        case Qt::Key::Key_W:
            m_left_joystick_y--;
            break;
        case Qt::Key::Key_D:
            m_left_joystick_x++;
            break;
        case Qt::Key::Key_S:
            m_left_joystick_y++;
            break;
        case Qt::Key::Key_A:
            m_left_joystick_x--;
            break;

        case Qt::Key::Key_Up:
            m_right_joystick_y--;
            break;
        case Qt::Key::Key_Right:
            m_right_joystick_x++;
            break;
        case Qt::Key::Key_Down:
            m_right_joystick_y++;
            break;
        case Qt::Key::Key_Left:
            m_right_joystick_x--;
            break;

        case Qt::Key::Key_Slash:
        case Qt::Key::Key_Question:
            m_buttons |= BUTTON_Y;
            break;
        case Qt::Key::Key_Shift:
        case Qt::Key::Key_Control:
            m_buttons |= BUTTON_B;
            break;
        case Qt::Key::Key_Enter:
        case Qt::Key::Key_Return:
            m_buttons |= BUTTON_A;
            break;
        case Qt::Key::Key_Apostrophe:
        case Qt::Key::Key_QuoteDbl:
            m_buttons |= BUTTON_X;
            break;
        case Qt::Key::Key_Q:
            m_buttons |= BUTTON_L;
            break;
        case Qt::Key::Key_E:
            m_buttons |= BUTTON_R;
            break;
        case Qt::Key::Key_R:
            m_buttons |= BUTTON_ZL;
            break;
        case Qt::Key::Key_Backslash:
            m_buttons |= BUTTON_ZR;
            break;
        case Qt::Key::Key_Equal:
            m_buttons |= BUTTON_PLUS;
            break;
        case Qt::Key::Key_Minus:
            m_buttons |= BUTTON_MINUS;
            break;
        case Qt::Key::Key_C:
            m_buttons |= BUTTON_LCLICK;
            break;
        case Qt::Key::Key_0:
            m_buttons |= BUTTON_RCLICK;
            break;
        case Qt::Key::Key_Home:
        case Qt::Key::Key_Escape:
            m_buttons |= BUTTON_HOME;
            break;
        case Qt::Key::Key_Insert:
            m_buttons |= BUTTON_CAPTURE;
            break;

        default:;
            return;
        }

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

        //  Suppress if key is already pressed.
        auto iter = m_pressed_buttons.find(key);
        if (iter == m_pressed_buttons.end()){
            return;
        }

        switch (key){
        case Qt::Key::Key_8:
            m_dpad_y++;
            break;
        case Qt::Key::Key_9:
            m_dpad_x--;
            m_dpad_y++;
            break;
        case Qt::Key::Key_6:
            m_dpad_x--;
            break;
        case Qt::Key::Key_3:
            m_dpad_x--;
            m_dpad_y--;
            break;
        case Qt::Key::Key_2:
            m_dpad_y--;
            break;
        case Qt::Key::Key_1:
            m_dpad_x++;
            m_dpad_y--;
            break;
        case Qt::Key::Key_4:
            m_dpad_x++;
            break;
        case Qt::Key::Key_7:
            m_dpad_x++;
            m_dpad_y++;
            break;

        case Qt::Key::Key_W:
            m_left_joystick_y++;
            break;
        case Qt::Key::Key_D:
            m_left_joystick_x--;
            break;
        case Qt::Key::Key_S:
            m_left_joystick_y--;
            break;
        case Qt::Key::Key_A:
            m_left_joystick_x++;
            break;

        case Qt::Key::Key_Up:
            m_right_joystick_y++;
            break;
        case Qt::Key::Key_Right:
            m_right_joystick_x--;
            break;
        case Qt::Key::Key_Down:
            m_right_joystick_y--;
            break;
        case Qt::Key::Key_Left:
            m_right_joystick_x++;
            break;

        case Qt::Key::Key_Slash:
        case Qt::Key::Key_Question:
            m_buttons &= ~BUTTON_Y;
            break;
        case Qt::Key::Key_Shift:
        case Qt::Key::Key_Control:
            m_buttons &= ~BUTTON_B;
            break;
        case Qt::Key::Key_Enter:
        case Qt::Key::Key_Return:
            m_buttons &= ~BUTTON_A;
            break;
        case Qt::Key::Key_Apostrophe:
        case Qt::Key::Key_QuoteDbl:
            m_buttons &= ~BUTTON_X;
            break;
        case Qt::Key::Key_Q:
            m_buttons &= ~BUTTON_L;
            break;
        case Qt::Key::Key_E:
            m_buttons &= ~BUTTON_R;
            break;
        case Qt::Key::Key_R:
            m_buttons &= ~BUTTON_ZL;
            break;
        case Qt::Key::Key_Backslash:
            m_buttons &= ~BUTTON_ZR;
            break;
        case Qt::Key::Key_Equal:
            m_buttons &= ~BUTTON_PLUS;
            break;
        case Qt::Key::Key_Minus:
            m_buttons &= ~BUTTON_MINUS;
            break;
        case Qt::Key::Key_C:
            m_buttons &= ~BUTTON_LCLICK;
            break;
        case Qt::Key::Key_0:
            m_buttons &= ~BUTTON_RCLICK;
            break;
        case Qt::Key::Key_Home:
        case Qt::Key::Key_Escape:
            m_buttons &= ~BUTTON_HOME;
            break;
        case Qt::Key::Key_Insert:
            m_buttons &= ~BUTTON_CAPTURE;
            break;

        default:;
            return;
        }

        m_pressed_buttons.erase(key);
        m_last = std::chrono::system_clock::now();
    }
//    print();

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}


void VirtualController::print(){
    cout << "dpad = (" << m_dpad_x << "," << m_dpad_y
         << "), left = (" << m_left_joystick_x << "," << m_left_joystick_y
         << "), right = (" << m_right_joystick_x << "," << m_right_joystick_y
         << "), buttons =";
    if (m_buttons & BUTTON_Y) cout << " Y";
    if (m_buttons & BUTTON_B) cout << " B";
    if (m_buttons & BUTTON_A) cout << " A";
    if (m_buttons & BUTTON_X) cout << " X";
    if (m_buttons & BUTTON_L) cout << " L";
    if (m_buttons & BUTTON_R) cout << " R";
    if (m_buttons & BUTTON_ZL) cout << " ZL";
    if (m_buttons & BUTTON_ZR) cout << " ZR";
    if (m_buttons & BUTTON_PLUS) cout << " +";
    if (m_buttons & BUTTON_MINUS) cout << " -";
    if (m_buttons & BUTTON_LCLICK) cout << " LC";
    if (m_buttons & BUTTON_RCLICK) cout << " RC";
    if (m_buttons & BUTTON_HOME) cout << " Home";
    if (m_buttons & BUTTON_CAPTURE) cout << " Screen";
    cout << ")" << endl;
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

            buttons = m_buttons;
            neutral = m_buttons == 0;

            dpad = DPAD_NONE;
            if (m_dpad_x != 0 || m_dpad_y != 0){
                neutral = false;
                do{
                    if (m_dpad_x == 0){
                        dpad = m_dpad_y > 0 ? DPAD_DOWN : DPAD_UP;
                        break;
                    }
                    if (m_dpad_y == 0){
                        dpad = m_dpad_x > 0 ? DPAD_RIGHT : DPAD_LEFT;
                        break;
                    }
                    if (m_dpad_x < 0 && m_dpad_y < 0){
                        dpad = DPAD_UP_LEFT;
                        break;
                    }
                    if (m_dpad_x < 0 && m_dpad_y > 0){
                        dpad = DPAD_DOWN_LEFT;
                        break;
                    }
                    if (m_dpad_x > 0 && m_dpad_y > 0){
                        dpad = DPAD_DOWN_RIGHT;
                        break;
                    }
                    if (m_dpad_x > 0 && m_dpad_y < 0){
                        dpad = DPAD_UP_RIGHT;
                        break;
                    }
                }while (false);
            }

            left_x = 128;
            left_y = 128;
            if (m_left_joystick_x != 0 || m_left_joystick_y != 0){
                neutral = false;
                int mag = std::abs(m_left_joystick_x) > std::abs(m_left_joystick_y)
                    ? std::abs(m_left_joystick_x)
                    : std::abs(m_left_joystick_y);
                left_x = std::min(128 * m_left_joystick_x / mag + 128, 255);
                left_y = std::min(128 * m_left_joystick_y / mag + 128, 255);
            }

            right_x = 128;
            right_y = 128;
            if (m_right_joystick_x != 0 || m_right_joystick_y != 0){
                neutral = false;
                int mag = std::abs(m_right_joystick_x) > std::abs(m_right_joystick_y)
                    ? std::abs(m_right_joystick_x)
                    : std::abs(m_right_joystick_y);
                right_x = std::min(128 * m_right_joystick_x / mag + 128, 255);
                right_y = std::min(128 * m_right_joystick_y / mag + 128, 255);
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
                pabb_controller_state params;
                params.button = buttons;
                params.dpad = dpad;
                params.dpad = dpad;
                params.left_joystick_x = left_x;
                params.left_joystick_y = left_y;
                params.right_joystick_x = right_x;
                params.right_joystick_y = right_y;
                params.ticks = m_granularity;
                while (m_botbase.try_send_request<PABB_MSG_CONTROLLER_STATE>(params));
            }catch (const char*){
            }catch (std::string&){
            }catch (PokemonAutomation::CancelledException&){}
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
                    "red"
                );
            }
        }else if (delays.size() > 10 && average < granularity / 2){
            if (m_granularity > 1){
                m_granularity--;
                m_logger.log(
                    "System sleep latency (" + QString::number(average.count()) + " ms). "
                    "Decreasing granularity to " + QString::number(m_granularity) + " ticks.",
                    "blue"
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





