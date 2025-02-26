/*  Nintendo Switch Pro Controller (SysbotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "SysbotBase_ProController.h"

#include "Common/Cpp/Concurrency/SpinPause.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



ProController_SysbotBase::ProController_SysbotBase(
    Logger& logger,
    SysbotBase::TcpSysbotBase_Connection& connection,
    const ControllerRequirements& requirements
)
    : ProControllerWithScheduler(logger, std::chrono::milliseconds(150))
    , m_connection(connection)
    , m_stopping(false)
    , m_replace_on_next(false)
    , m_command_queue(QUEUE_SIZE)
    , m_is_active(false)
{
    if (!connection.is_ready()){
        return;
    }

    //  Check compatibility.

    const std::map<ControllerType, std::set<ControllerFeature>>& controllers = connection.supported_controllers();
    auto iter = controllers.find(ControllerType::NintendoSwitch_WiredProController);

    std::string missing_feature;
    do{
        if (iter == controllers.end()){
            missing_feature = "NintendoSwitch_WiredProController";
            break;
        }

        missing_feature = requirements.check_compatibility(iter->second);

        if (PreloadSettings::instance().DEVELOPER_MODE &&
            missing_feature == CONTROLLER_FEATURE_STRINGS.get_string(ControllerFeature::TickPrecise)
        ){
            logger.log("Bypassing Missing Requirement: " + missing_feature, COLOR_RED);
            missing_feature.clear();
        }

        if (missing_feature.empty()){
            m_dispatch_thread = std::thread(&ProController_SysbotBase::thread_body, this);
            return;
        }

    }while (false);

    m_error_string = html_color_text("Missing Feature: " + missing_feature, COLOR_RED);

}
ProController_SysbotBase::~ProController_SysbotBase(){
    stop();
    m_stopping.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lg(m_state_lock);
        m_cv.notify_all();
    }
    if (m_dispatch_thread.joinable()){
        m_dispatch_thread.join();
    }
}



void ProController_SysbotBase::cancel_all_commands(){
//    cout << "ProController_SysbotBase::cancel_all_commands()" << endl;
    std::lock_guard<std::mutex> lg(m_state_lock);
    size_t queue_size = m_command_queue.size();
    if (queue_size > 0){
        m_command_queue.clear();
        m_is_active = false;
        m_cv.notify_all();
    }
    this->clear_on_next();
    m_logger.log("cancel_all_commands(): Command Queue Size = " + std::to_string(queue_size), COLOR_DARKGREEN);
}
void ProController_SysbotBase::replace_on_next_command(){
//    cout << "ProController_SysbotBase::replace_on_next_command - Enter()" << endl;
    std::lock_guard<std::mutex> lg(m_state_lock);
    m_cv.notify_all();
    m_replace_on_next = true;
    this->clear_on_next();
    m_logger.log("replace_on_next_command(): Command Queue Size = " + std::to_string(m_command_queue.size()), COLOR_DARKGREEN);
}


void ProController_SysbotBase::wait_for_all(const Cancellable* cancellable){
//    cout << "ProController_SysbotBase::wait_for_all - Enter()" << endl;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    std::unique_lock<std::mutex> lg1(m_state_lock);
    m_logger.log("wait_for_all(): Command Queue Size = " + std::to_string(m_command_queue.size()), COLOR_DARKGREEN);
    this->issue_wait_for_all(cancellable);
    m_cv.wait(lg1, [this]{
        return m_command_queue.empty() || m_replace_on_next;
    });
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
//    cout << "ProController_SysbotBase::wait_for_all - Exit()" << endl;
}
void ProController_SysbotBase::push_state(const Cancellable* cancellable, WallDuration duration){
    //  Must be called inside "m_state_lock".

    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (!is_ready()){
        throw InvalidConnectionStateException();
    }

    Button buttons = BUTTON_NONE;
    for (size_t c = 0; c < 14; c++){
        buttons |= m_buttons[c].is_busy()
            ? (Button)((uint16_t)1 << c)
            : BUTTON_NONE;
    }

    DpadPosition dpad = m_dpad.is_busy() ? m_dpad.position : DPAD_NONE;

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

    std::unique_lock<std::mutex> lg(m_state_lock, std::adopt_lock_t());

    m_cv.wait(lg, [this]{
        return m_command_queue.size() < QUEUE_SIZE || m_replace_on_next;
    });

    lg.release();

    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    if (m_replace_on_next){
        m_command_queue.clear();
        m_is_active = false;
        m_replace_on_next = false;
    }

    if (m_command_queue.empty()){
        m_cv.notify_all();
    }

    Command& command = m_command_queue.push_back();

    command.state.buttons = buttons;
    command.state.dpad = dpad;
    command.state.left_x = left_x;
    command.state.left_y = left_y;
    command.state.right_x = right_x;
    command.state.right_y = right_y;

    command.duration = std::chrono::duration_cast<Milliseconds>(duration);
}


void ProController_SysbotBase::send_botbase_request(
    const Cancellable* cancellable,
    const BotBaseRequest& request
){
    throw InternalProgramError(
        &m_logger, PA_CURRENT_FUNCTION,
        "ProController_SysbotBase::send_botbase_request(): Not supported."
    );
}
BotBaseMessage ProController_SysbotBase::send_botbase_request_and_wait(
    const Cancellable* cancellable,
    const BotBaseRequest& request
){
    throw InternalProgramError(
        &m_logger, PA_CURRENT_FUNCTION,
        "ProController_SysbotBase::send_botbase_request_and_wait(): Not supported."
    );
}


struct SplitDpad{
    bool up = false;
    bool right = false;
    bool down = false;
    bool left = false;
};

SplitDpad convert_unified_to_split_dpad(DpadPosition dpad){
    switch (dpad){
    case DpadPosition::DPAD_UP:
        return {true, false, false, false};
    case DpadPosition::DPAD_UP_RIGHT:
        return {true, true, false, false};
    case DpadPosition::DPAD_RIGHT:
        return {false, true, false, false};
    case DpadPosition::DPAD_DOWN_RIGHT:
        return {false, true, true, false};
    case DpadPosition::DPAD_DOWN:
        return {false, false, true, false};
    case DpadPosition::DPAD_DOWN_LEFT:
        return {false, false, true, true};
    case DpadPosition::DPAD_LEFT:
        return {false, false, false, true};
    case DpadPosition::DPAD_UP_LEFT:
        return {true, false, false, true};
    default:
        return {false, false, false, false};
    }
}


void ProController_SysbotBase::send_diff(
    const SwitchControllerState& old_state,
    const SwitchControllerState& new_state
){
#if 0
    m_logger.log(
        "send_diff(): (" + button_to_string(new_state.buttons) +
        "), dpad(" + dpad_to_string(new_state.dpad) +
        "), LJ(" + std::to_string(new_state.left_x) + "," + std::to_string(new_state.left_y) +
        "), RJ(" + std::to_string(new_state.right_x) + "," + std::to_string(new_state.right_y) +
        ")",
        COLOR_DARKGREEN
    );
#endif

    //  These need to match:
    //  https://github.com/olliz0r/sys-botbase/blob/master/sys-botbase/source/util.c#L145
    static const std::string BUTTON_MAP[] = {
        "Y", "B", "A", "X",
        "L", "R", "ZL", "ZR",
        "MINUS", "PLUS", "LSTICK", "RSTICK",
        "HOME", "CAPTURE"
    };

    std::string message;

    if (old_state.buttons != new_state.buttons){
        for (size_t c = 0; c < 14; c++){
            uint16_t mask = (uint16_t)1 << c;
            bool before = (uint16_t)old_state.buttons & mask;
            bool after = (uint16_t)new_state.buttons & mask;
            if (before == after){
                continue;
            }
            if (after){
                message += "press " + BUTTON_MAP[c] + "\n";
            }else{
                message += "release " + BUTTON_MAP[c] + "\n";
            }
        }
    }

    if (old_state.dpad != new_state.dpad){
        SplitDpad old_dpad = convert_unified_to_split_dpad(old_state.dpad);
        SplitDpad new_dpad = convert_unified_to_split_dpad(new_state.dpad);
        if (old_dpad.up != new_dpad.up){
            message += new_dpad.up ? "press DU\n" : "release DU\n";
        }
        if (old_dpad.right != new_dpad.right){
            message += new_dpad.right ? "press DR\n" : "release DR\n";
        }
        if (old_dpad.down != new_dpad.down){
            message += new_dpad.down ? "press DD\n" : "release DD\n";
        }
        if (old_dpad.left != new_dpad.left){
            message += new_dpad.left ? "press DL\n" : "release DL\n";
        }
    }

    if (old_state.left_x != new_state.left_x ||
        old_state.left_y != new_state.left_y
    ){
        message += "setStick LEFT ";
        message += std::to_string(((uint16_t)new_state.left_x - 128) << 8);
        message += " ";
        message += std::to_string(((uint16_t)128 - new_state.left_y) << 8);
        message += "\n";
    }
    if (old_state.right_x != new_state.right_x ||
        old_state.right_y != new_state.right_y
    ){
        message += "setStick RIGHT ";
        message += std::to_string(((uint16_t)new_state.right_x - 128) << 8);
        message += " ";
        message += std::to_string(((uint16_t)128 - new_state.right_y) << 8);
        message += "\n";
    }

    if (message.empty()){
        return;
    }

//    cout << message << endl;
    m_connection.write_data(message);

    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("sys-botbase: " + message);
    }
}


void ProController_SysbotBase::thread_body(){
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread();
    std::chrono::microseconds EARLY_WAKE = GlobalSettings::instance().PERFORMANCE->PRECISE_WAKE_MARGIN;

    SwitchControllerState current_state;

    std::unique_lock<std::mutex> lg(m_state_lock);
    while (!m_stopping.load(std::memory_order_relaxed)){
        if (m_command_queue.empty()){
            m_is_active = false;
            if (current_state.is_neutral()){
                m_cv.wait(lg);
                continue;
            }

            send_diff(current_state, SwitchControllerState());
            current_state.clear();
            continue;
        }

        WallClock now = current_time();

        //  Check the next item in the schedule.
        Command& command = m_command_queue.front();

        //  Waking up from idle.
        if (!m_is_active){
            m_is_active = true;
            m_queue_start_time = now;

            send_diff(current_state, command.state);
            current_state = command.state;

            WallClock expiration = m_queue_start_time + command.duration;
            m_cv.wait_until(lg, expiration - EARLY_WAKE);
            pause();
            continue;
        }

        //  Already running.
        WallClock expiration = m_queue_start_time + command.duration;

        //  Current command hasn't expired yet.
        if (now < expiration){
            m_cv.wait_until(lg, expiration - EARLY_WAKE);
            pause();
            continue;
        }

        //  Command has expired. We can pop it.
        m_is_active = false;
        m_queue_start_time = expiration;
        m_command_queue.pop_front();
        m_cv.notify_all();

        //  Check how much we shot passed the expiration time.
//        WallDuration delay = now - expiration;

//        cout << "Delay = " << std::chrono::duration_cast<std::chrono::microseconds>(delay) << endl;
    }

    SwitchControllerState neutral_state;
    send_diff(current_state, neutral_state);
}























}
}
