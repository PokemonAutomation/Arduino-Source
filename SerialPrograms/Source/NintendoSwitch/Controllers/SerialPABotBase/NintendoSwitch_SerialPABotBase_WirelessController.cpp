/*  SerialPABotBase: Wireless Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_ESP32.h"
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







void SerialPABotBase_WirelessController::issue_report(
    const Cancellable* cancellable,
    const SerialPABotBase::NintendoSwitch_ButtonState& buttons,
    WallDuration duration
){
    //  Release the state lock since we are no longer touching state.
    //  This loop can block indefinitely if the command queue is full.
    ReverseLockGuard<std::mutex> lg(m_state_lock);

    //  We will not do any throttling or timing adjustments here. We'll defer
    //  to the microcontroller to do that for us.

    //  Divide the controller state into smaller chunks of 65535 milliseconds.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);

//    time_left = (time_left + 14ms) / 15ms * 15ms;

//    time_left = std::max(time_left, 15ms);
    while (time_left > Milliseconds::zero()){
        Milliseconds current = std::min(time_left, 65535ms);
        time_left -= current;

#if 0
        //  Make sure the last block isn't too small.
        if (0ms < time_left && time_left < 15ms){
            time_left += current;
            current = time_left / 2;
            time_left -= current;
        }
#endif
//        cout << "current = " << current.count() << endl;

        m_serial->issue_request(
            SerialPABotBase::MessageControllerStateButtons(
                (uint16_t)current.count(),
                buttons
            ),
            cancellable
        );
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
        BotBaseMessage response = m_serial->issue_request_and_wait(
            SerialPABotBase::MessageControllerGetColors(m_controller_type),
            &m_scope
        );

        using ControllerColors = SerialPABotBase::NintendoSwitch_ControllerColors;
        ControllerColors colors{};
        if (response.body.size() == sizeof(seqnum_t) + sizeof(ControllerColors)){
            memcpy(&colors, response.body.data() + sizeof(seqnum_t), sizeof(ControllerColors));
        }else{
            m_logger.log(
                "Invalid response size to PABB_MSG_ESP32_GET_COLORS: body = " + std::to_string(response.body.size()),
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
            bool status_connected   = status & 1;
            bool status_paired      = status & 2;

            std::string str;
            str += "Connected: " + (status_connected
                ? html_color_text("Yes", theme_friendly_darkblue())
                : html_color_text("No", COLOR_RED)
            );
            str += " - Paired: " + (status_paired
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
