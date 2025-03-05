/*  SerialPABotBase: Wireless Pro Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <sstream>
#include <iostream>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_ESP32.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/ControllerCapability.h"
#include "NintendoSwitch_SerialPABotBase_WirelessProController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;



SerialPABotBase_WirelessProController::SerialPABotBase_WirelessProController(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    const ControllerRequirements& requirements
)
    : SerialPABotBase_ProController(
        logger,
        ControllerType::NintendoSwitch_WirelessProController,
        connection,
        requirements
    )
    , m_stopping(false)
    , m_status_thread(&SerialPABotBase_WirelessProController::status_thread, this)
{}
SerialPABotBase_WirelessProController::~SerialPABotBase_WirelessProController(){
    stop();
    m_status_thread.join();
}
void SerialPABotBase_WirelessProController::stop(){
    if (m_stopping.exchange(true)){
        return;
    }
    SerialPABotBase_ProController::stop();
    m_scope.cancel(nullptr);
    {
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        if (m_serial){
            m_serial->notify_all();
        }
        m_cv.notify_all();
    }
}




class SerialPABotBase_WirelessProController::MessageControllerStatus : public BotBaseRequest{
public:
    pabb_esp32_RequestStatus params;
    MessageControllerStatus()
        : BotBaseRequest(false)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_REQUEST_STATUS, params);
    }
};
class SerialPABotBase_WirelessProController::MessageControllerState : public BotBaseRequest{
public:
    pabb_esp32_report30 params;
    MessageControllerState(uint8_t ticks, bool active, ESP32Report0x30 report)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.ticks = ticks;
        params.active = active;
        params.report = report;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_REPORT, params);
    }
};

int register_message_converters_ESP32(){
    register_message_converter(
        PABB_MSG_ESP32_REQUEST_STATUS,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "ESP32_controller_status() - ";
            if (body.size() != sizeof(pabb_esp32_RequestStatus)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_esp32_RequestStatus*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_ESP32_REPORT,
        [](const std::string& body){
            //  Disable this by default since it's very spammy.
            if (!GlobalSettings::instance().LOG_EVERYTHING){
                return std::string();
            }
            std::ostringstream ss;
            ss << "ESP32_controller_state() - ";
            if (body.size() != sizeof(pabb_esp32_report30)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_esp32_report30*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", ticks = " << (int)params->ticks;
            ss << ", active = " << (int)params->active;
            return ss.str();
        }
    );
    return 0;
}
int init_Messages_ESP32 = register_message_converters_ESP32();



void SerialPABotBase_WirelessProController::push_state(const Cancellable* cancellable, WallDuration duration){
    //  https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md

    bool is_active = this->is_active();

    ESP32Report0x30 report{
        .report_id = 0x30,
        .timer = 0,     //  Populate on controller.
        .byte2 = 0x91,  //  Full + charging : Pro controller + USB powered
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .leftstick_x_lo = 0x00,
        .leftstick_x_hi = 0x08,
        .leftstick_y = 0x80,
        .rightstick_x_lo = 0x00,
        .rightstick_x_hi = 0x08,
        .rightstick_y = 0x80,
        .vibrator = 0x80,
        .gyro = {},
    };


    for (size_t c = 0; c < TOTAL_BUTTONS; c++){
        if (!m_buttons[c].is_busy()){
            continue;
        }
        Button button = (Button)((ButtonFlagType)1 << c);
        switch (button){
        //  Right
        case BUTTON_Y:          report.button3 |= 1 << 0; break;
        case BUTTON_X:          report.button3 |= 1 << 1; break;
        case BUTTON_B:          report.button3 |= 1 << 2; break;
        case BUTTON_A:          report.button3 |= 1 << 3; break;
//        case BUTTON_RIGHT_SR:   report.button3 |= 1 << 4; break;
//        case BUTTON_RIGHT_SL:   report.button3 |= 1 << 5; break;
        case BUTTON_R:          report.button3 |= 1 << 6; break;
        case BUTTON_ZR:         report.button3 |= 1 << 7; break;

        //  Shared
        case BUTTON_MINUS:      report.button4 |= 1 << 0; break;
        case BUTTON_PLUS:       report.button4 |= 1 << 1; break;
        case BUTTON_RCLICK:     report.button4 |= 1 << 2; break;
        case BUTTON_LCLICK:     report.button4 |= 1 << 3; break;
        case BUTTON_HOME:       report.button4 |= 1 << 4; break;
        case BUTTON_CAPTURE:    report.button4 |= 1 << 5; break;

        //  Left
        case BUTTON_DOWN:       report.button5 |= 1 << 0; break;
        case BUTTON_UP:         report.button5 |= 1 << 1; break;
        case BUTTON_RIGHT:      report.button5 |= 1 << 2; break;
        case BUTTON_LEFT:       report.button5 |= 1 << 3; break;
//        case BUTTON_LEFT_SR:    report.button5 |= 1 << 4; break;
//        case BUTTON_LEFT_SL:    report.button5 |= 1 << 5; break;
        case BUTTON_L:          report.button5 |= 1 << 6; break;
        case BUTTON_ZL:         report.button5 |= 1 << 7; break;

        default:;
        }
    }

    if (m_dpad.is_busy()){
        SplitDpad dpad = convert_unified_to_split_dpad(m_dpad.position);
        report.button5 |= (dpad.down  ? 1 : 0) << 0;
        report.button5 |= (dpad.up    ? 1 : 0) << 1;
        report.button5 |= (dpad.right ? 1 : 0) << 2;
        report.button5 |= (dpad.left  ? 1 : 0) << 3;
    }

    //  Left Stick
    if (m_left_joystick.is_busy()){
        report.leftstick_x_lo = (m_left_joystick.x << 4) & 0xf0;
        report.leftstick_x_hi = (m_left_joystick.x & 0xf0) >> 4;
        report.leftstick_y = 255 - m_left_joystick.y;
    }

    //  Right Stick
    if (m_right_joystick.is_busy()){
        report.rightstick_x_lo = (m_right_joystick.x << 4) & 0xf0;
        report.rightstick_x_hi = (m_right_joystick.x & 0xf0) >> 4;
        report.rightstick_y = 255 - m_right_joystick.y;
    }


    //  Release the state lock since we are no longer touching state.
    //  This loop can block indefinitely if the command queue is full.
    ReverseLockGuard<std::mutex> lg(m_state_lock);

    //  Divide the controller state into smaller chunks of 255 ticks.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);
    while (time_left > Milliseconds::zero()){
        Milliseconds current_ms = std::min(time_left, 255 * 15ms);
        uint8_t current_ticks = (uint8_t)milliseconds_to_ticks_15ms(current_ms.count());
        m_serial->issue_request(
            MessageControllerState(current_ticks, is_active, report),
            cancellable
        );
        time_left -= current_ms;
    }
}


void SerialPABotBase_WirelessProController::status_thread(){
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

    WallClock next_ping = current_time();
    while (true){
        if (m_stopping.load(std::memory_order_relaxed) || !m_handle.is_ready()){
            break;
        }

        std::string error;
        try{
            pabb_MsgAckRequestI32 response;
            m_serial->issue_request_and_wait(
                MessageControllerStatus(),
                &m_scope
            ).convert<PABB_MSG_ACK_REQUEST_I32>(logger(), response);
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
