/*  SerialPABotBase: Pokken Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "Common/SerialPABotBase/SerialPABotBase_Protocol_IDs.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_Protocol.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_NS_Generic.h"
#include "NintendoSwitch_SerialPABotBase_PokkenController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;




SerialPABotBase_PokkenController::SerialPABotBase_PokkenController(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection
)
    : SerialPABotBase_Controller(
        logger,
        ControllerType::NintendoSwitch_WiredProController,
        connection
    )
    , m_use_milliseconds(m_supported_features.contains(ControllerFeature::TimingFlexibleMilliseconds))
    , m_stopping(false)
    , m_status_thread(&SerialPABotBase_PokkenController::status_thread, this)
{}
SerialPABotBase_PokkenController::~SerialPABotBase_PokkenController(){
    stop();
    m_status_thread.join();
}
void SerialPABotBase_PokkenController::stop(){
    if (m_stopping.exchange(true)){
        return;
    }
    ProController::stop();
    m_scope.cancel(nullptr);
    {
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        if (m_serial){
            m_serial->notify_all();
        }
        m_cv.notify_all();
    }
}





void SerialPABotBase_PokkenController::push_state(const Cancellable* cancellable, WallDuration duration){
    //  Must be called inside "m_state_lock".

    if (!is_ready()){
        throw InvalidConnectionStateException();
    }

    int dpad_x = 0;
    int dpad_y = 0;
    uint16_t buttons = 0;
    for (size_t c = 0; c < TOTAL_BUTTONS; c++){
        if (!m_buttons[c].is_busy()){
            continue;
        }
        Button button = (Button)((ButtonFlagType)1 << c);
        switch (button){
        case BUTTON_Y:          buttons |= 1 <<  0; break;
        case BUTTON_B:          buttons |= 1 <<  1; break;
        case BUTTON_A:          buttons |= 1 <<  2; break;
        case BUTTON_X:          buttons |= 1 <<  3; break;
        case BUTTON_L:          buttons |= 1 <<  4; break;
        case BUTTON_R:          buttons |= 1 <<  5; break;
        case BUTTON_ZL:         buttons |= 1 <<  6; break;
        case BUTTON_ZR:         buttons |= 1 <<  7; break;
        case BUTTON_MINUS:      buttons |= 1 <<  8; break;
        case BUTTON_PLUS:       buttons |= 1 <<  9; break;
        case BUTTON_LCLICK:     buttons |= 1 << 10; break;
        case BUTTON_RCLICK:     buttons |= 1 << 11; break;
        case BUTTON_HOME:       buttons |= 1 << 12; break;
        case BUTTON_CAPTURE:    buttons |= 1 << 13; break;
        case BUTTON_UP:         dpad_y--; break;
        case BUTTON_RIGHT:      dpad_x++; break;
        case BUTTON_DOWN:       dpad_y++; break;
        case BUTTON_LEFT:       dpad_x--; break;
        default:;
        }
    }


    //  Merge the dpad states.
    DpadPosition dpad = m_dpad.is_busy() ? m_dpad.position : DPAD_NONE;
    {
        switch (dpad){
        case DpadPosition::DPAD_UP:
            dpad_y--;
            break;
        case DpadPosition::DPAD_UP_RIGHT:
            dpad_x++;
            dpad_y--;
            break;
        case DpadPosition::DPAD_RIGHT:
            dpad_x++;
            break;
        case DpadPosition::DPAD_DOWN_RIGHT:
            dpad_x++;
            dpad_y++;
            break;
        case DpadPosition::DPAD_DOWN:
            dpad_y++;
            break;
        case DpadPosition::DPAD_DOWN_LEFT:
            dpad_x--;
            dpad_y++;
            break;
        case DpadPosition::DPAD_LEFT:
            dpad_x--;
            break;
        case DpadPosition::DPAD_UP_LEFT:
            dpad_x--;
            dpad_y--;
            break;
        default:;
        }

        if (dpad_x < 0){
            if (dpad_y < 0){
                dpad = DpadPosition::DPAD_UP_LEFT;
            }else if (dpad_y > 0){
                dpad = DpadPosition::DPAD_DOWN_LEFT;
            }else{
                dpad = DpadPosition::DPAD_LEFT;
            }
        }else if (dpad_x > 0){
            if (dpad_y < 0){
                dpad = DpadPosition::DPAD_UP_RIGHT;
            }else if (dpad_y > 0){
                dpad = DpadPosition::DPAD_DOWN_RIGHT;
            }else{
                dpad = DpadPosition::DPAD_RIGHT;
            }
        }else{
            if (dpad_y < 0){
                dpad = DpadPosition::DPAD_UP;
            }else if (dpad_y > 0){
                dpad = DpadPosition::DPAD_DOWN;
            }else{
                dpad = DPAD_NONE;
            }
        }
    }



    uint8_t left_x = 128;
    uint8_t left_y = 128;
    uint8_t right_x = 128;
    uint8_t right_y = 128;
    if (m_left_joystick.is_busy()){
        left_x = m_left_joystick.x;
        left_y = m_left_joystick.y;
    }
    if (m_right_joystick.is_busy()){
        right_x = m_right_joystick.x;
        right_y = m_right_joystick.y;
    }


    //  Release the state lock since we are no longer touching state.
    //  This loop can block indefinitely if the command queue is full.
    ReverseLockGuard<std::mutex> lg(m_state_lock);

    //  Divide the controller state into smaller chunks that fit into the report
    //  duration.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);

    if (m_use_milliseconds){
        while (time_left > Milliseconds::zero()){
            Milliseconds current = std::min(time_left, 65535ms);
            m_serial->issue_request(
                SerialPABotBase::DeviceRequest_NS_Generic_ControllerStateMs(
                    (uint16_t)current.count(),
                    buttons,
                    dpad,
                    left_x, left_y,
                    right_x, right_y
                ),
                cancellable
            );
            time_left -= current;
        }
    }else{
        while (time_left > Milliseconds::zero()){
            Milliseconds current_ms = std::min(time_left, 255 * 8ms);
            uint8_t current_ticks = (uint8_t)milliseconds_to_ticks_8ms(current_ms.count());
            m_serial->issue_request(
                SerialPABotBase::DeviceRequest_NS_Generic_ControllerStateTicks(
                    buttons,
                    dpad,
                    left_x, left_y,
                    right_x, right_y,
                    current_ticks
                ),
                cancellable
            );
            time_left -= current_ms;
        }
    }
}


//
//  Given a regularly reported 32-bit counter that wraps around, infer its true
//  64-bit value.
//
class ExtendedLengthCounter{
public:
    ExtendedLengthCounter()
        : m_high_bits(0)
        , m_last_received(0)
    {}

    uint64_t push_short_value(uint32_t counter){
        if (counter < m_last_received && counter - m_last_received < 0x40000000){
            m_high_bits++;
        }
        m_last_received = counter;
        return ((uint64_t)m_high_bits << 32) | counter;
    }

private:
    uint32_t m_high_bits;
    uint32_t m_last_received;
};



void SerialPABotBase_PokkenController::status_thread(){
    constexpr std::chrono::milliseconds PERIOD(1000);
    std::atomic<WallClock> last_ack(current_time());

    std::thread watchdog([&, this]{
        WallClock next_ping = current_time();
        while (true){
            if (m_stopping.load(std::memory_order_relaxed) || !m_handle.is_ready()){
                break;
            }

            auto last = current_time() - last_ack.load(std::memory_order_relaxed);
            std::chrono::duration<double> seconds = last;
            if (last > 2 * PERIOD){
                std::string text = "Last Ack: " + tostr_fixed(seconds.count(), 3) + " seconds ago";
                m_handle.set_status_line1(text, COLOR_RED);
//                m_logger.log("Connection issue detected. Turning on all logging...");
//                settings.log_everything.store(true, std::memory_order_release);
            }

            std::unique_lock<std::mutex> lg(m_sleep_lock);
            if (m_stopping.load(std::memory_order_relaxed) || !m_handle.is_ready()){
                break;
            }

            WallClock now = current_time();
            next_ping += PERIOD;
            if (now + PERIOD < next_ping){
                next_ping = now + PERIOD;
            }
            m_cv.wait_until(lg, next_ping);
        }
    });


    ExtendedLengthCounter clock_tracker;
    WallClock next_ping = current_time();
    while (true){
        if (m_stopping.load(std::memory_order_relaxed) || !m_handle.is_ready()){
            break;
        }

        std::string error;
        try{
            pabb_MsgAckRequestI32 response;
            m_serial->issue_request_and_wait(
                SerialPABotBase::DeviceRequest_system_clock(),
                &m_scope
            ).convert<PABB_MSG_ACK_REQUEST_I32>(logger(), response);
            last_ack.store(current_time(), std::memory_order_relaxed);
            uint64_t wallclock = clock_tracker.push_short_value(response.data);

            if (wallclock == 0){
                m_handle.set_status_line1(
                    "Not connected to Switch.",
                    COLOR_RED
                );
            }else{
                m_handle.set_status_line1(
                    "Up Time: " + ticks_to_time(
                        m_use_milliseconds
                            ? 1000
                            : NintendoSwitch::TICKS_PER_SECOND,
                        wallclock
                    ),
                    theme_friendly_darkblue()
                );
            }
        }catch (OperationCancelledException&){
            break;
        }catch (InvalidConnectionStateException&){
            break;
        }catch (SerialProtocolException& e){
            error = e.message();
        }catch (ConnectionException& e){
            error = e.message();
        }catch (...){
            error = "Unknown error.";
        }
        if (!error.empty()){
            m_handle.set_status_line1(error, COLOR_RED);
        }

//        cout << "lock()" << endl;
        std::unique_lock<std::mutex> lg(m_sleep_lock);
//        cout << "lock() - done" << endl;
        if (m_stopping.load(std::memory_order_relaxed) || !m_handle.is_ready()){
            break;
        }

        WallClock now = current_time();
        next_ping += PERIOD;
        if (now + PERIOD < next_ping){
            next_ping = now + PERIOD;
        }
        m_cv.wait_until(lg, next_ping);
    }

    {
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        m_cv.notify_all();
    }
    watchdog.join();
}






}
}
