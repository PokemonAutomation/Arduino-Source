/*  SerialPABotBase: Wireless Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_Protocol.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_ESP32.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_SerialPABotBase_WirelessController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;







SerialPABotBase_WirelessController::SerialPABotBase_WirelessController(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerType controller_type
)
    : SerialPABotBase_Controller(
        logger,
        controller_type,
        connection
    )
    , m_controller_type(controller_type)
    , m_timing_variation(ConsoleSettings::instance().TIMING_OPTIONS.WIRELESS_ESP32)
    , m_stopping(false)
    , m_status_thread(&SerialPABotBase_WirelessController::status_thread, this)
{}
SerialPABotBase_WirelessController::~SerialPABotBase_WirelessController(){
    stop();
    m_status_thread.join();
}
void SerialPABotBase_WirelessController::stop(){
    if (m_stopping.exchange(true)){
        return;
    }
    m_scope.cancel(nullptr);
    {
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        if (m_serial){
            m_serial->notify_all();
        }
        m_cv.notify_all();
    }
}





Button SerialPABotBase_WirelessController::populate_report_buttons(pa_NintendoSwitch_WirelessController_State0x30_Buttons& buttons){
    //  https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md

    Button all_buttons = BUTTON_NONE;
    for (size_t c = 0; c < TOTAL_BUTTONS; c++){
        if (!m_buttons[c].is_busy()){
            continue;
        }
        Button button = (Button)((ButtonFlagType)1 << c);
        all_buttons |= button;
        switch (button){
        //  Right
        case BUTTON_Y:          buttons.button3 |= 1 << 0; break;
        case BUTTON_X:          buttons.button3 |= 1 << 1; break;
        case BUTTON_B:          buttons.button3 |= 1 << 2; break;
        case BUTTON_A:          buttons.button3 |= 1 << 3; break;
        case BUTTON_RIGHT_SR:   buttons.button3 |= 1 << 4; break;
        case BUTTON_RIGHT_SL:   buttons.button3 |= 1 << 5; break;
        case BUTTON_R:          buttons.button3 |= 1 << 6; break;
        case BUTTON_ZR:         buttons.button3 |= 1 << 7; break;

        //  Shared
        case BUTTON_MINUS:      buttons.button4 |= 1 << 0; break;
        case BUTTON_PLUS:       buttons.button4 |= 1 << 1; break;
        case BUTTON_RCLICK:     buttons.button4 |= 1 << 2; break;
        case BUTTON_LCLICK:     buttons.button4 |= 1 << 3; break;
        case BUTTON_HOME:       buttons.button4 |= 1 << 4; break;
        case BUTTON_CAPTURE:    buttons.button4 |= 1 << 5; break;

        //  Left
        case BUTTON_DOWN:       buttons.button5 |= 1 << 0; break;
        case BUTTON_UP:         buttons.button5 |= 1 << 1; break;
        case BUTTON_RIGHT:      buttons.button5 |= 1 << 2; break;
        case BUTTON_LEFT:       buttons.button5 |= 1 << 3; break;
        case BUTTON_LEFT_SR:    buttons.button5 |= 1 << 4; break;
        case BUTTON_LEFT_SL:    buttons.button5 |= 1 << 5; break;
        case BUTTON_L:          buttons.button5 |= 1 << 6; break;
        case BUTTON_ZL:         buttons.button5 |= 1 << 7; break;

        default:;
        }
    }
    return all_buttons;
}
bool SerialPABotBase_WirelessController::populate_report_gyro(pa_NintendoSwitch_WirelessController_State0x30_Gyro& gyro){
    bool gyro_active = false;
    {
        if (m_accel_x.is_busy()){
            gyro.accel_x = m_accel_x.value;
            gyro_active = true;
        }
        if (m_accel_y.is_busy()){
            gyro.accel_y = m_accel_y.value;
            gyro_active = true;
        }
        if (m_accel_z.is_busy()){
            gyro.accel_z = m_accel_z.value;
            gyro_active = true;
        }
        if (m_rotation_x.is_busy()){
            gyro.rotation_x = m_rotation_x.value;
            gyro_active = true;
        }
        if (m_rotation_y.is_busy()){
            gyro.rotation_y = m_rotation_y.value;
            gyro_active = true;
        }
        if (m_rotation_z.is_busy()){
            gyro.rotation_z = m_rotation_z.value;
            gyro_active = true;
        }
    }
    return gyro_active;
}


void SerialPABotBase_WirelessController::issue_report(
    const Cancellable* cancellable,
    WallDuration duration,
    const pa_NintendoSwitch_WirelessController_State0x30_Buttons& buttons
){
    //  Release the state lock since we are no longer touching state.
    //  This loop can block indefinitely if the command queue is full.
    ReverseLockGuard<std::mutex> lg(m_state_lock);

    //  We will not do any throttling or timing adjustments here. We'll defer
    //  to the microcontroller to do that for us.

    //  Divide the controller state into smaller chunks of 65535 milliseconds.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);

    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        m_serial->issue_request(
            SerialPABotBase::MessageControllerStateButtons(
                (uint16_t)current.count(),
                buttons
            ),
            cancellable
        );
        time_left -= current;
    }
}
void SerialPABotBase_WirelessController::issue_report(
    const Cancellable* cancellable,
    WallDuration duration,
    const pa_NintendoSwitch_WirelessController_State0x30_Buttons& buttons,
    const pa_NintendoSwitch_WirelessController_State0x30_Gyro& gyro
){
    //  Release the state lock since we are no longer touching state.
    //  This loop can block indefinitely if the command queue is full.
    ReverseLockGuard<std::mutex> lg(m_state_lock);

    //  TODO: For now we duplicate the gyro data to all 3 5ms segments.
    pa_NintendoSwitch_WirelessController_State0x30_GyroX3 gyro3{
        gyro, gyro, gyro
    };

#if 0
    //  Purturb results to show the Switch that they are not stuck.
    if (gyro3.time1.accel_x > 0){
        gyro3.time0.accel_x--;
        gyro3.time2.accel_x--;
    }else if (gyro3.time1.accel_x < 0){
        gyro3.time0.accel_x++;
        gyro3.time2.accel_x++;
    }else{
        gyro3.time0.accel_x--;
        gyro3.time2.accel_x++;
    }
    if (gyro3.time1.accel_y > 0){
        gyro3.time0.accel_y--;
        gyro3.time2.accel_y--;
    }else if (gyro3.time1.accel_y < 0){
        gyro3.time0.accel_y++;
        gyro3.time2.accel_y++;
    }else{
        gyro3.time0.accel_y--;
        gyro3.time2.accel_y++;
    }
    if (gyro3.time1.accel_z > 0){
        gyro3.time0.accel_z--;
        gyro3.time2.accel_z--;
    }else if (gyro3.time1.accel_z < 0){
        gyro3.time0.accel_z++;
        gyro3.time2.accel_z++;
    }else{
        gyro3.time0.accel_z--;
        gyro3.time2.accel_z++;
    }
#endif

    //  We will not do any throttling or timing adjustments here. We'll defer
    //  to the microcontroller to do that for us.

    //  Divide the controller state into smaller chunks of 65535 milliseconds.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);

    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        m_serial->issue_request(
            SerialPABotBase::MessageControllerStateFull(
                (uint16_t)current.count(),
                buttons, gyro3
            ),
            cancellable
        );
        time_left -= current;
    }
}




void SerialPABotBase_WirelessController::status_thread(){
    constexpr std::chrono::milliseconds PERIOD(1000);
    std::atomic<WallClock> last_ack(current_time());

    //  Read controller colors.
    std::string color_html;
#if 1
    try{
        m_logger.log("Reading Controller Colors...");

        using ControllerColors = PABB_NintendoSwitch_ControllerColors;

        BotBaseMessage response = m_serial->issue_request_and_wait(
            SerialPABotBase::MessageControllerReadSpi(
                m_controller_type,
                0x00006050, sizeof(ControllerColors)
            ),
            &m_scope
        );

        ControllerColors colors{};
        if (response.body.size() == sizeof(seqnum_t) + sizeof(ControllerColors)){
            memcpy(&colors, response.body.data() + sizeof(seqnum_t), sizeof(ControllerColors));
        }else{
            m_logger.log(
                "Invalid response size to PABB_MSG_ESP32_REQUEST_READ_SPI: body = " + std::to_string(response.body.size()),
                COLOR_RED
            );
            m_handle.set_status_line1("Error: See log for more information.", COLOR_RED);
            return;
        }
        m_logger.log("Reading Controller Colors... Done");

        switch (m_controller_type){
        case ControllerType::NintendoSwitch_WirelessProController:{
            Color left(colors.left_grip[0], colors.left_grip[1], colors.left_grip[2]);
            Color body(colors.body[0], colors.body[1], colors.body[2]);
            Color right(colors.right_grip[0], colors.right_grip[1], colors.right_grip[2]);
            color_html += html_color_text("&#x2b24;", left);
            color_html += " " + html_color_text("&#x2b24;", body);
            color_html += " " + html_color_text("&#x2b24;", right);
            break;
        }
        case ControllerType::NintendoSwitch_LeftJoycon:
        case ControllerType::NintendoSwitch_RightJoycon:{
            Color body(colors.body[0], colors.body[1], colors.body[2]);
            color_html = html_color_text("&#x2b24;", body);
            break;
        }
        default:;
        }

    }catch (Exception& e){
        e.log(m_logger);
        m_handle.set_status_line1("Error: See log for more information.", COLOR_RED);
        return;
    }
#endif


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

    WallClock next_ping = current_time();
    while (true){
        if (m_stopping.load(std::memory_order_relaxed) || !m_handle.is_ready()){
            break;
        }

        std::string error;
        try{
            pabb_MsgAckRequestI32 response;
            m_serial->issue_request_and_wait(
                SerialPABotBase::MessageControllerStatus(),
                &m_scope
            ).convert<PABB_MSG_ACK_REQUEST_I32>(m_logger, response);
            last_ack.store(current_time(), std::memory_order_relaxed);

            uint32_t status = response.data;
            bool status_connected = status & 1;
            bool status_ready     = status & 2;

            std::string str;
            str += "Connected: " + (status_connected
                ? html_color_text("Yes", theme_friendly_darkblue())
                : html_color_text("No", COLOR_RED)
            );
            str += " - Ready: " + (status_ready
                ? html_color_text("Yes", theme_friendly_darkblue())
                : html_color_text("No", COLOR_RED)
            );
            str += " - " + color_html;

            m_handle.set_status_line1(str);
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
            stop();
            m_handle.set_status_line1(error, COLOR_RED);
            break;
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
