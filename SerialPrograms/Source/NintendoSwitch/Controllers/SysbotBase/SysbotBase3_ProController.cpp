/*  Nintendo Switch Pro Controller (SysbotBase 3)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
//#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Controllers/JoystickTools.h"
#include "SysbotBase3_ControllerState.h"
#include "SysbotBase3_ProController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



ProController_SysbotBase3::ProController_SysbotBase3(
    Logger& logger,
    SysbotBase::TcpSysbotBase_Connection& connection
)
    : ProController(logger)
    , ControllerWithScheduler(logger)
    , m_connection(connection)
    , m_stopping(false)
    , m_pending_replace(false)
    , m_next_seqnum(1)
    , m_next_expected_seqnum_ack(1)
{
    if (!connection.is_ready()){
        return;
    }
    connection.add_listener(*this);
}
ProController_SysbotBase3::~ProController_SysbotBase3(){
    m_connection.remove_listener(*this);
}

const ControllerFeatures& ProController_SysbotBase3::controller_features() const{
    static const ControllerFeatures features{
        ControllerFeature::TickPrecise,
        ControllerFeature::NintendoSwitch_ProController,
        ControllerFeature::NintendoSwitch_DateSkip,
    };
    return features;
}

void ProController_SysbotBase3::cancel_all_commands(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (m_stopping){
        throw InvalidConnectionStateException("");
    }

    uint64_t queued = m_next_seqnum - m_next_expected_seqnum_ack;
    m_next_expected_seqnum_ack = m_next_seqnum;

    m_connection.write_data("cqCancel\r\n");
    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("sys-botbase3: cqCancel");
    }

    this->clear_on_next();
    m_cv.notify_all();
    m_logger.log("cancel_all_commands(): Command Queue Size = " + std::to_string(queued), COLOR_DARKGREEN);
}
void ProController_SysbotBase3::replace_on_next_command(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (m_stopping){
        throw InvalidConnectionStateException("");
    }

    uint64_t queued = m_next_seqnum - m_next_expected_seqnum_ack;
    m_next_expected_seqnum_ack = m_next_seqnum;

    m_pending_replace = true;

#if 0
    m_connection.write_data("cqReplaceOnNext\r\n");
    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("sys-botbase3: cqReplaceOnNext");
    }
#endif

    this->clear_on_next();
    m_cv.notify_all();
    m_logger.log("replace_on_next_command(): Command Queue Size = " + std::to_string(queued), COLOR_DARKGREEN);
}
void ProController_SysbotBase3::wait_for_all(const Cancellable* cancellable){
    std::unique_lock<std::mutex> lg(m_state_lock);
    while (true){
        if (m_stopping){
            throw InvalidConnectionStateException("");
        }
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        if (m_next_seqnum == m_next_expected_seqnum_ack){
            break;
        }
        m_cv.wait(lg);
    }
}


void ProController_SysbotBase3::on_message(const std::string& message){
    const std::string TOKEN = "cqCommandFinished";
    auto pos = message.find(TOKEN);
    if (pos == std::string::npos){
        return;
    }

    const char* ptr = message.c_str();
    ptr += pos;
    ptr += TOKEN.size();
    uint64_t parsed;
    while (true){
        char ch = *ptr;
        switch (ch){
        case '\0':
            return;
        case ' ':
        case '\t':
            ptr++;
            continue;
        }
        parsed = std::atoll(ptr);
        break;
    }

    std::lock_guard<std::mutex> lg(m_state_lock);

//    cout << "parsed = " << parsed << endl;
//    cout << "m_next_seqnum = " << m_next_seqnum << endl;
//    cout << "m_next_expected_seqnum_ack = " << m_next_expected_seqnum_ack << endl;

    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log(
            "Command Finished: " + std::to_string(parsed) +
            " (queue size = " + std::to_string(m_next_seqnum - m_next_expected_seqnum_ack) + ")"
        );
    }

    //  Old ack
    if (parsed < m_next_expected_seqnum_ack){
        m_logger.log(
            "Received Old Ack: Expected = " + std::to_string(m_next_expected_seqnum_ack) +
            ", Actual = " + std::to_string(parsed),
            COLOR_RED
        );
        return;
    }

    //  Ack is ahead of what has been dispatched.
    if (parsed >= m_next_seqnum){
        m_logger.log(
            "Received Future Ack: Expected = " + std::to_string(m_next_expected_seqnum_ack) +
            ", Actual = " + std::to_string(parsed),
            COLOR_RED
        );
        return;
    }

    m_next_expected_seqnum_ack = parsed + 1;
    m_cv.notify_all();
}

void ProController_SysbotBase3::push_state(const Cancellable* cancellable, WallDuration duration){
    //  Must be called inside "m_state_lock".

    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_stopping){
        throw InvalidConnectionStateException("");
    }

    //  Flags map to: https://github.com/switchbrew/libnx/blob/master/nx/include/switch/services/hid.h#L584
    uint64_t nx_button = 0;
    if (m_buttons[ 0].is_busy()) nx_button |= (uint64_t)1 <<  3;    //  Y
    if (m_buttons[ 1].is_busy()) nx_button |= (uint64_t)1 <<  1;    //  B
    if (m_buttons[ 2].is_busy()) nx_button |= (uint64_t)1 <<  0;    //  A
    if (m_buttons[ 3].is_busy()) nx_button |= (uint64_t)1 <<  2;    //  X
    if (m_buttons[ 4].is_busy()) nx_button |= (uint64_t)1 <<  6;    //  L
    if (m_buttons[ 5].is_busy()) nx_button |= (uint64_t)1 <<  7;    //  R
    if (m_buttons[ 6].is_busy()) nx_button |= (uint64_t)1 <<  8;    //  ZL
    if (m_buttons[ 7].is_busy()) nx_button |= (uint64_t)1 <<  9;    //  ZR
    if (m_buttons[ 8].is_busy()) nx_button |= (uint64_t)1 << 11;    //  -
    if (m_buttons[ 9].is_busy()) nx_button |= (uint64_t)1 << 10;    //  +
    if (m_buttons[10].is_busy()) nx_button |= (uint64_t)1 <<  4;    //  L-click
    if (m_buttons[11].is_busy()) nx_button |= (uint64_t)1 <<  5;    //  R-click
    if (m_buttons[12].is_busy()) nx_button |= (uint64_t)1 << 18;    //  Home
    if (m_buttons[13].is_busy()) nx_button |= (uint64_t)1 << 19;    //  Capture
    if (m_buttons[14].is_busy()) nx_button |= (uint64_t)1 << 13;    //  Up
    if (m_buttons[15].is_busy()) nx_button |= (uint64_t)1 << 14;    //  Right
    if (m_buttons[16].is_busy()) nx_button |= (uint64_t)1 << 15;    //  Down
    if (m_buttons[17].is_busy()) nx_button |= (uint64_t)1 << 12;    //  Left
#if 0   //  Don't exist on pro controller.
    if (m_buttons[18].is_busy()) nx_button |= (uint64_t)1 << 24;    //  Left SL
    if (m_buttons[19].is_busy()) nx_button |= (uint64_t)1 << 25;    //  Left SR
    if (m_buttons[20].is_busy()) nx_button |= (uint64_t)1 << 26;    //  Right SL
    if (m_buttons[21].is_busy()) nx_button |= (uint64_t)1 << 27;    //  Right SR
#endif

    {
        DpadPosition dpad = m_dpad.is_busy() ? m_dpad.position : DPAD_NONE;
        SplitDpad split_dpad = convert_unified_to_split_dpad(dpad);
        if (split_dpad.up)    nx_button |= (uint64_t)1 << 13;
        if (split_dpad.right) nx_button |= (uint64_t)1 << 14;
        if (split_dpad.down)  nx_button |= (uint64_t)1 << 15;
        if (split_dpad.left)  nx_button |= (uint64_t)1 << 12;
    }

    int16_t left_x = 0;
    int16_t left_y = 0;
    int16_t right_x = 0;
    int16_t right_y = 0;
    if (m_left_joystick.is_busy()){
        double fx = JoystickTools::linear_u8_to_float(m_left_joystick.x);
        double fy = -JoystickTools::linear_u8_to_float(m_left_joystick.y);
        JoystickTools::clip_magnitude(fx, fy);
        left_x = JoystickTools::linear_float_to_s16(fx);
        left_y = JoystickTools::linear_float_to_s16(fy);
    }
    if (m_right_joystick.is_busy()){
        double fx = JoystickTools::linear_u8_to_float(m_right_joystick.x);
        double fy = -JoystickTools::linear_u8_to_float(m_right_joystick.y);
        JoystickTools::clip_magnitude(fx, fy);
        right_x = JoystickTools::linear_float_to_s16(fx);
        right_y = JoystickTools::linear_float_to_s16(fy);
    }

    std::string message;
    if (m_pending_replace){
        m_pending_replace = false;
        message += "cqReplaceOnNext\r\n";
    }

    std::unique_lock<std::mutex> lg(m_state_lock, std::adopt_lock_t());

    //  Wait until there is space.
    m_cv.wait(lg, [this, cancellable]{
        if (cancellable && cancellable->cancelled()){
            return true;
        }
        return m_stopping || m_next_seqnum - m_next_expected_seqnum_ack < QUEUE_SIZE;
    });

    lg.release();

    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    Sysbotbase3_ControllerCommand command;
    command.milliseconds = std::chrono::duration_cast<Milliseconds>(duration).count();
    command.seqnum = m_next_seqnum++;
    command.state.buttons = nx_button;
    command.state.left_joystick_x = left_x;
    command.state.left_joystick_y = left_y;
    command.state.right_joystick_x = right_x;
    command.state.right_joystick_y = right_y;

    {
        std::string command_message;
        command_message.resize(64);
        command.write_to_hex(command_message.data());
        message += "cqControllerState " + command_message + "\r\n";
    }

    m_connection.write_data(message);

    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("sys-botbase3: " + message);
    }
}







}
}
