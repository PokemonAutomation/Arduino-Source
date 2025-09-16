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

    m_scheduler.clear_on_next();
    m_cv.notify_all();
    m_logger.log("cancel_all_commands(): Command Queue Size = " + std::to_string(queued), COLOR_DARKGREEN);
}
void ProController_SysbotBase3::replace_on_next_command(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (m_stopping){
        throw InvalidConnectionStateException("");
    }

    uint64_t queued = m_next_seqnum - m_next_expected_seqnum_ack;
//    m_next_expected_seqnum_ack = m_next_seqnum;

    m_pending_replace = true;

#if 0
    m_connection.write_data("cqReplaceOnNext\r\n");
    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("sys-botbase3: cqReplaceOnNext");
    }
#endif

    m_scheduler.clear_on_next();
    m_cv.notify_all();
    m_logger.log("replace_on_next_command(): Command Queue Size = " + std::to_string(queued), COLOR_DARKGREEN);
}
void ProController_SysbotBase3::wait_for_all(const Cancellable* cancellable){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);

    //    cout << "wait_for_all() - start" << endl;

        if (m_stopping){
            throw InvalidConnectionStateException("");
        }
        m_scheduler.issue_wait_for_all(schedule);
    }
    execute_schedule(cancellable, schedule);

    std::unique_lock<std::mutex> lg1(m_state_lock);
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
        m_cv.wait(lg1);
    }

//    cout << "wait_for_all() - done" << endl;
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
            COLOR_DARKGREEN
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

void ProController_SysbotBase3::execute_state(
    const Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (m_stopping){
        throw InvalidConnectionStateException("");
    }

    SwitchControllerState controller_state;
    for (auto& item : entry.state){
        static_cast<const SwitchCommand&>(*item).apply(controller_state);
    }


    //  Flags map to: https://github.com/switchbrew/libnx/blob/master/nx/include/switch/services/hid.h#L584
    uint64_t nx_button = 0;
    if (controller_state.buttons & BUTTON_Y)        nx_button |= (uint64_t)1 <<  3;    //  Y
    if (controller_state.buttons & BUTTON_B)        nx_button |= (uint64_t)1 <<  1;    //  B
    if (controller_state.buttons & BUTTON_A)        nx_button |= (uint64_t)1 <<  0;    //  A
    if (controller_state.buttons & BUTTON_X)        nx_button |= (uint64_t)1 <<  2;    //  X
    if (controller_state.buttons & BUTTON_L)        nx_button |= (uint64_t)1 <<  6;    //  L
    if (controller_state.buttons & BUTTON_R)        nx_button |= (uint64_t)1 <<  7;    //  R
    if (controller_state.buttons & BUTTON_ZL)       nx_button |= (uint64_t)1 <<  8;    //  ZL
    if (controller_state.buttons & BUTTON_ZR)       nx_button |= (uint64_t)1 <<  9;    //  ZR
    if (controller_state.buttons & BUTTON_MINUS)    nx_button |= (uint64_t)1 << 11;    //  -
    if (controller_state.buttons & BUTTON_PLUS)     nx_button |= (uint64_t)1 << 10;    //  +
    if (controller_state.buttons & BUTTON_LCLICK)   nx_button |= (uint64_t)1 <<  4;    //  L-click
    if (controller_state.buttons & BUTTON_RCLICK)   nx_button |= (uint64_t)1 <<  5;    //  R-click
    if (controller_state.buttons & BUTTON_HOME)     nx_button |= (uint64_t)1 << 18;    //  Home
    if (controller_state.buttons & BUTTON_CAPTURE)  nx_button |= (uint64_t)1 << 19;    //  Capture
    if (controller_state.buttons & BUTTON_UP)       nx_button |= (uint64_t)1 << 13;    //  Up
    if (controller_state.buttons & BUTTON_RIGHT)    nx_button |= (uint64_t)1 << 14;    //  Right
    if (controller_state.buttons & BUTTON_DOWN)     nx_button |= (uint64_t)1 << 15;    //  Down
    if (controller_state.buttons & BUTTON_LEFT)     nx_button |= (uint64_t)1 << 12;    //  Left
#if 0   //  Don't exist on pro controller.
    if (controller_state.buttons & BUTTON_LEFT_SL)  nx_button |= (uint64_t)1 << 24;    //  Left SL
    if (controller_state.buttons & BUTTON_LEFT_SR)  nx_button |= (uint64_t)1 << 25;    //  Left SR
    if (controller_state.buttons & BUTTON_RIGHT_SL) nx_button |= (uint64_t)1 << 26;    //  Right SL
    if (controller_state.buttons & BUTTON_RIGHT_SR) nx_button |= (uint64_t)1 << 27;    //  Right SR
#endif

    {
        DpadPosition dpad = controller_state.dpad;
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
    {
        double fx = JoystickTools::linear_u8_to_float(controller_state.left_stick_x);
        double fy = -JoystickTools::linear_u8_to_float(controller_state.left_stick_y);
        JoystickTools::clip_magnitude(fx, fy);
        left_x = JoystickTools::linear_float_to_s16(fx);
        left_y = JoystickTools::linear_float_to_s16(fy);
    }
    {
        double fx = JoystickTools::linear_u8_to_float(controller_state.right_stick_x);
        double fy = -JoystickTools::linear_u8_to_float(controller_state.right_stick_y);
        JoystickTools::clip_magnitude(fx, fy);
        right_x = JoystickTools::linear_float_to_s16(fx);
        right_y = JoystickTools::linear_float_to_s16(fy);
    }

    std::string message;
    if (m_pending_replace){
        m_pending_replace = false;
        m_next_expected_seqnum_ack = m_next_seqnum;
        message += "cqReplaceOnNext\r\n";
    }

    //  Wait until there is space.
    std::unique_lock<std::mutex> lg(m_state_lock);
    m_cv.wait(lg, [this, cancellable]{
        if (cancellable && cancellable->cancelled()){
            return true;
        }
        return m_stopping || m_next_seqnum - m_next_expected_seqnum_ack < QUEUE_SIZE;
    });

    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    Sysbotbase3_ControllerCommand command;
    command.milliseconds = std::chrono::duration_cast<Milliseconds>(entry.duration).count();
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

    //  Do not log the contents of the command due to privacy concerns.
    //  (people entering passwords)
#if 0
    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("sys-botbase3: " + message);
    }
#endif
}







}
}
