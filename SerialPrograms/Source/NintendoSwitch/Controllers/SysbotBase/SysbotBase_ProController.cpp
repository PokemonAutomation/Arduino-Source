/*  Nintendo Switch Pro Controller (SysbotBase)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/SpinPause.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "Controllers/JoystickTools.h"
#include "SysbotBase_ProController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



ProController_SysbotBase::ProController_SysbotBase(
    Logger& logger,
    SysbotBase::TcpSysbotBase_Connection& connection
)
    : ProController(logger)
    , ControllerWithScheduler(logger)
    , m_connection(connection)
    , m_stopping(false)
    , m_replace_on_next(false)
    , m_command_queue(QUEUE_SIZE)
    , m_next_state_change(WallClock::max())
{
    if (!connection.is_ready()){
        return;
    }

    m_dispatch_thread = std::thread(&ProController_SysbotBase::thread_body, this);
}
ProController_SysbotBase::~ProController_SysbotBase(){
    stop();
    if (m_dispatch_thread.joinable()){
        m_dispatch_thread.join();
    }
}
void ProController_SysbotBase::stop(){
    if (m_stopping.exchange(true)){
        return;
    }
    ProController::stop();
    {
        std::lock_guard<std::mutex> lg(m_state_lock);
        m_cv.notify_all();
    }
}



void ProController_SysbotBase::cancel_all_commands(){
//    cout << "ProController_SysbotBase::cancel_all_commands()" << endl;
    std::lock_guard<std::mutex> lg(m_state_lock);
    size_t queue_size = m_command_queue.size();
    m_next_state_change = WallClock::min();
    m_command_queue.clear();
    m_cv.notify_all();
    m_scheduler.clear_on_next();
    m_logger.log("cancel_all_commands(): Command Queue Size = " + std::to_string(queue_size), COLOR_DARKGREEN);
}
void ProController_SysbotBase::replace_on_next_command(){
//    cout << "ProController_SysbotBase::replace_on_next_command - Enter()" << endl;
    std::lock_guard<std::mutex> lg(m_state_lock);
    m_cv.notify_all();
    m_replace_on_next = true;
    m_scheduler.clear_on_next();
    m_logger.log("replace_on_next_command(): Command Queue Size = " + std::to_string(m_command_queue.size()), COLOR_DARKGREEN);
}


void ProController_SysbotBase::wait_for_all(const Cancellable* cancellable){
//    cout << "ProController_SysbotBase::wait_for_all - Enter()" << endl;
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);
        m_logger.log("wait_for_all(): Command Queue Size = " + std::to_string(m_command_queue.size()), COLOR_DARKGREEN);
        m_scheduler.issue_wait_for_all(schedule);
    }
    execute_schedule(cancellable, schedule);

    std::unique_lock<std::mutex> lg1(m_state_lock);
    m_cv.wait(lg1, [this]{
        return m_next_state_change == WallClock::max() || m_replace_on_next;
    });
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
//    cout << "ProController_SysbotBase::wait_for_all - Exit()" << endl;
}
void ProController_SysbotBase::execute_state(
    const Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
    if (!is_ready()){
        throw InvalidConnectionStateException("");
    }

    SwitchControllerState controller_state;
    for (auto& item : entry.state){
        static_cast<const SwitchCommand&>(*item).apply(controller_state);
    }

    //  Wait until there is space.
    std::unique_lock<std::mutex> lg(m_state_lock);
    m_cv.wait(lg, [this]{
        return m_command_queue.size() < QUEUE_SIZE || m_replace_on_next;
    });


    if (cancellable){
        cancellable->throw_if_cancelled();
    }

    if (m_replace_on_next){
//        cout << "executing replace" << endl;
        m_replace_on_next = false;
        m_command_queue.clear();
        m_next_state_change = WallClock::min();
        m_cv.notify_all();
    }

    //  Enqueuing into empty+idle queue.
    if (m_next_state_change == WallClock::max()){
        m_next_state_change = WallClock::min();
        m_cv.notify_all();
    }

    Command& command = m_command_queue.push_back();

    command.state.buttons = controller_state.buttons;
    command.state.dpad = controller_state.dpad;
    command.state.left_x = controller_state.left_stick_x;
    command.state.left_y = controller_state.left_stick_y;
    command.state.right_x = controller_state.right_stick_x;
    command.state.right_y = controller_state.right_stick_y;

    command.duration = std::chrono::duration_cast<Milliseconds>(entry.duration);
}



void ProController_SysbotBase::send_diff(
    const ProControllerState& old_state,
    const ProControllerState& new_state
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
    static const std::vector<std::pair<Button, std::string>> BUTTON_MAP{
        {BUTTON_Y, "Y"},
        {BUTTON_B, "B"},
        {BUTTON_A, "A"},
        {BUTTON_X, "X"},
        {BUTTON_L, "L"},
        {BUTTON_R, "R"},
        {BUTTON_ZL, "ZL"},
        {BUTTON_ZR, "ZR"},
        {BUTTON_MINUS, "MINUS"},
        {BUTTON_PLUS, "PLUS"},
        {BUTTON_LCLICK, "LSTICK"},
        {BUTTON_RCLICK, "RSTICK"},
        {BUTTON_HOME, "HOME"},
        {BUTTON_CAPTURE, "CAPTURE"},
        {BUTTON_UP, "DU"},
        {BUTTON_RIGHT, "DR"},
        {BUTTON_DOWN, "DD"},
        {BUTTON_LEFT, "DL"},
    };

    std::string message;


    //  Merge the dpad states.
    ButtonFlagType old_buttons = old_state.buttons;
    ButtonFlagType new_buttons = new_state.buttons;

    SplitDpad old_dpad = convert_unified_to_split_dpad(old_state.dpad);
    if (old_dpad.up)    old_buttons |= BUTTON_UP;
    if (old_dpad.right) old_buttons |= BUTTON_RIGHT;
    if (old_dpad.down)  old_buttons |= BUTTON_DOWN;
    if (old_dpad.left)  old_buttons |= BUTTON_LEFT;

    SplitDpad new_dpad = convert_unified_to_split_dpad(new_state.dpad);
    if (new_dpad.up)    new_buttons |= BUTTON_UP;
    if (new_dpad.right) new_buttons |= BUTTON_RIGHT;
    if (new_dpad.down)  new_buttons |= BUTTON_DOWN;
    if (new_dpad.left)  new_buttons |= BUTTON_LEFT;


    if (old_buttons != new_buttons){
        for (const auto& button : BUTTON_MAP){
            ButtonFlagType mask = (ButtonFlagType)button.first;
            bool before = (ButtonFlagType)old_buttons & mask;
            bool after = (ButtonFlagType)new_buttons & mask;
            if (before == after){
                continue;
            }
            if (after){
                message += "press " + button.second + "\r\n";
            }else{
                message += "release " + button.second + "\r\n";
            }
        }
    }

    if (old_state.left_x != new_state.left_x ||
        old_state.left_y != new_state.left_y
    ){
        double fx = JoystickTools::linear_u8_to_float(new_state.left_x);
        double fy = -JoystickTools::linear_u8_to_float(new_state.left_y);
        JoystickTools::clip_magnitude(fx, fy);
//        cout << "fx = " << fx << ", fy = " << fy << endl;
        int16_t ix = JoystickTools::linear_float_to_s16(fx);
        int16_t iy = JoystickTools::linear_float_to_s16(fy);
//        cout << "ix = " << ix << ", iy = " << iy << endl;
        message += "setStick LEFT ";
        message += std::to_string(ix);
        message += " ";
        message += std::to_string(iy);
        message += "\r\n";
    }
    if (old_state.right_x != new_state.right_x ||
        old_state.right_y != new_state.right_y
    ){
        double fx = JoystickTools::linear_u8_to_float(new_state.right_x);
        double fy = -JoystickTools::linear_u8_to_float(new_state.right_y);
        JoystickTools::clip_magnitude(fx, fy);
        int16_t ix = JoystickTools::linear_float_to_s16(fx);
        int16_t iy = JoystickTools::linear_float_to_s16(fy);
        message += "setStick RIGHT ";
        message += std::to_string(ix);
        message += " ";
        message += std::to_string(iy);
        message += "\r\n";
    }

    if (message.empty()){
        return;
    }

//    cout << message << endl;
    m_connection.write_data(message);

    //  Do not log the contents of the command due to privacy concerns.
    //  (people entering passwords)
#if 0
    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("sys-botbase: " + message);
    }
#endif
}


void ProController_SysbotBase::thread_body(){
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread(m_logger);
    std::chrono::microseconds EARLY_WAKE = GlobalSettings::instance().PERFORMANCE->PRECISE_WAKE_MARGIN;

    ProControllerState current_state;

    std::unique_lock<std::mutex> lg(m_state_lock);
    while (!m_stopping.load(std::memory_order_relaxed)){
        WallClock now = current_time();

        //  State change.
        if (now >= m_next_state_change){
            if (m_command_queue.empty()){
                send_diff(current_state, ProControllerState());
                current_state.clear();
                m_next_state_change = WallClock::max();
            }else{
                Command& command = m_command_queue.front();
                send_diff(current_state, command.state);
                current_state = command.state;
                if (m_next_state_change == WallClock::min()){
                    m_next_state_change = now;
                }
                m_next_state_change += command.duration;
                m_command_queue.pop_front();
            }
            m_cv.notify_all();
            continue;
        }

        if (now + EARLY_WAKE >= m_next_state_change){
            pause();
            continue;
        }

        m_cv.wait_until(lg, m_next_state_change - EARLY_WAKE);

        //  Check how much we shot passed the expiration time.
//        WallDuration delay = now - expiration;

//        cout << "Delay = " << std::chrono::duration_cast<std::chrono::microseconds>(delay) << endl;
    }

    ProControllerState neutral_state;
    send_diff(current_state, neutral_state);
}























}
}
