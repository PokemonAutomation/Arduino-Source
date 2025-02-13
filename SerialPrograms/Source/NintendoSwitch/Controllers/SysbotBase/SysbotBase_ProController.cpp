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

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


#if 0
template <typename LockType, typename TimePoint>
auto precise_wait_until(
    std::condition_variable& cv,
    std::unique_lock<LockType>& lock,
    TimePoint wake_time
){
    cv.wait_until(lock, wake_time - std::chrono::microseconds(500));
    while (std::chrono::system_clock::now() < wake_time){
        pause();
    }
}
#endif


namespace NintendoSwitch{



ProController_SysbotBase::ProController_SysbotBase(
    Logger& logger,
    SysbotBase::SysbotBaseNetwork_Connection& connection,
    const ControllerRequirements& requirements
)
    : ProControllerWithScheduler(logger)
    , m_connection(connection)
    , m_stopping(false)
    , m_replace_on_next(false)
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

        if (missing_feature.empty()){
            m_dispatch_thread = std::thread(&ProController_SysbotBase::thread_body, this);
            return;
        }

    }while (false);

    m_error_string = html_color_text("Missing Feature: " + missing_feature, COLOR_RED);
}
ProController_SysbotBase::~ProController_SysbotBase(){
    m_stopping.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_cv.notify_all();
    }
    if (m_dispatch_thread.joinable()){
        m_dispatch_thread.join();
    }
}




void ProController_SysbotBase::wait_for_all(const Cancellable* cancellable){
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [this]{
        return m_command_queue.empty();
    });
}
void ProController_SysbotBase::cancel_all_commands(const Cancellable* cancellable){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_command_queue.empty()){
        return;
    }
    m_command_queue.clear();
    m_cv.notify_all();
}
void ProController_SysbotBase::replace_on_next_command(const Cancellable* cancellable){
    m_replace_on_next.store(true, std::memory_order_relaxed);
}


void ProController_SysbotBase::issue_controller_state(
    const Cancellable* cancellable,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    Milliseconds duration
){
    std::unique_lock<std::mutex> lg(m_lock);
//    cout << "issue_controller_state()" << endl;

    if (m_replace_on_next.load(std::memory_order_acquire)){
        m_command_queue.clear();
        m_replace_on_next.store(false, std::memory_order_relaxed);
    }

    m_cv.wait(lg, [this]{
        return m_command_queue.size() < QUEUE_SIZE;
    });

    if (m_command_queue.empty()){
        m_cv.notify_all();
    }

    Command& command = m_command_queue.emplace_back();

    command.state.buttons = button;
    command.state.dpad = position;
    command.state.left_x = left_x;
    command.state.left_y = left_y;
    command.state.right_x = right_x;
    command.state.right_y = right_y;

    command.duration = duration;
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



void ProController_SysbotBase::send_diff(
    const SwitchControllerState& old_state,
    const SwitchControllerState& new_state
){
    //  TODO
}


void ProController_SysbotBase::thread_body(){
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread();

    bool is_active = false;
    SwitchControllerState current_state;

    std::unique_lock<std::mutex> lg(m_lock);
    while (!m_stopping.load(std::memory_order_relaxed)){
        if (m_command_queue.empty()){
            is_active = false;
            if (current_state.is_neutral()){
                m_cv.wait(lg);
                continue;
            }

            send_diff(current_state, SwitchControllerState());
            current_state.clear();
            m_logger.log(
                "sys-botbase: (" + button_to_string(current_state.buttons) +
                "), dpad(" + dpad_to_string(current_state.dpad) +
                "), LJ(" + std::to_string(current_state.left_x) + "," + std::to_string(current_state.left_y) +
                "), RJ(" + std::to_string(current_state.right_x) + "," + std::to_string(current_state.right_y) +
                ")",
                COLOR_DARKGREEN
            );
            continue;
        }

        WallClock now = current_time();

        //  Check the next item in the schedule.
        Command& command = m_command_queue.front();

        //  Waking up from idle.
        if (!is_active){
            //  Waking up from idle.
            is_active = true;
            m_queue_start_time = now;

            send_diff(current_state, command.state);
            current_state = command.state;
            m_logger.log(
                "sys-botbase: (" + button_to_string(current_state.buttons) +
                "), dpad(" + dpad_to_string(current_state.dpad) +
                "), LJ(" + std::to_string(current_state.left_x) + "," + std::to_string(current_state.left_y) +
                "), RJ(" + std::to_string(current_state.right_x) + "," + std::to_string(current_state.right_y) +
                ")",
                COLOR_DARKGREEN
            );

            WallClock expiration = m_queue_start_time + command.duration;
            m_cv.wait_until(lg, expiration - EARLY_WAKE_SPIN);
            pause();
            continue;
        }

        //  Already running.
        WallClock expiration = m_queue_start_time + command.duration;

        //  Current command hasn't expired yet.
        if (now < expiration){
            m_cv.wait_until(lg, expiration - EARLY_WAKE_SPIN);
            pause();
            continue;
        }

        //  Command has expired. We can pop it.
        m_queue_start_time = expiration;
        m_command_queue.pop_front();
        m_cv.notify_all();

        //  Check how much we shot passed the expiration time.
//        WallDuration delay = now - expiration;

//        cout << "Delay = " << std::chrono::duration_cast<std::chrono::microseconds>(delay) << endl;
    }
}























}
}
