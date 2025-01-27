/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/Microcontroller/MessageProtocol.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "Controllers/GlobalQtKeyMap.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "NintendoSwitch_VirtualControllerMapping.h"
#include "NintendoSwitch_VirtualController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;





VirtualController::VirtualController(
    Logger& logger,
    ControllerSession& session,
    bool allow_commands_while_running
)
    : m_logger(logger)
    , m_session(session)
    , m_allow_commands_while_running(allow_commands_while_running)
    , m_last_known_state(ProgramState::STOPPED)
    , m_stop(false)
    , m_thread(run_with_catch, "VirtualController::thread_loop()", [this]{ thread_loop(); })
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
    {
        WriteSpinLock lg(m_state_lock);
        m_state_tracker.clear();
    }

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
}

bool VirtualController::on_key_press(const QKeyEvent& key){
//    cout << "press: " << key.key() << ", native = " << key.nativeVirtualKey() << endl;
    QtKeyMap::instance().record(key);
    {
        WriteSpinLock lg(m_state_lock);
        m_state_tracker.press(key.nativeVirtualKey());
    }

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
    return true;
}
bool VirtualController::on_key_release(const QKeyEvent& key){
//    cout << "release: " << key.key() << ", native = " << key.nativeVirtualKey() << endl;
    QtKeyMap::instance().record(key);
    {
        WriteSpinLock lg(m_state_lock);
        m_state_tracker.release(key.nativeVirtualKey());
    }

    std::lock_guard<std::mutex> lg(m_sleep_lock);
    m_cv.notify_all();
    return true;
}


bool VirtualController::try_stop_commands(){
    std::string error = m_session.try_run<SwitchController>([](SwitchController& controller){
        controller.cancel_all(nullptr);
    });
    return error.empty();
}
bool VirtualController::try_next_interrupt(){
    std::string error = m_session.try_run<SwitchController>([](SwitchController& controller){
        controller.replace_on_next_command(nullptr);
    });
    return error.empty();
}


void VirtualController::thread_loop(){
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread();

    VirtualControllerState last;
    bool last_neutral = true;
    WallClock last_press = current_time();
    while (true){
        if (m_stop.load(std::memory_order_acquire)){
            return;
        }

        //  Not accepting commands.
        if (!m_allow_commands_while_running &&
            m_last_known_state.load(std::memory_order_acquire) != ProgramState::STOPPED
        ){
            last = VirtualControllerState();
            std::unique_lock<std::mutex> lg(m_sleep_lock);
            if (m_stop.load(std::memory_order_acquire)){
                return;
            }
            m_cv.wait(lg);
            continue;
        }

        //  Get the raw keyboard state.
        std::set<uint32_t> pressed_native_keys;
        WallClock next_wake;
        {
            ReadSpinLock lg(m_state_lock);
            pressed_native_keys = m_state_tracker.get_currently_pressed();
            next_wake = m_state_tracker.next_state_change();
        }

#if 0
        //  REMOVE
        std::string str = "state: ";
        for (uint32_t native_key : pressed_native_keys){
            str += std::to_string(native_key);
            str += " ";
        }
        cout << str << endl;    //  REMOVE
#endif

        //  Convert the raw keyboard state to controller state.
        VirtualControllerState current;
        {
            const QtKeyMap& qkey_map = QtKeyMap::instance();
            for (uint32_t native_key : pressed_native_keys){
                const std::set<Qt::Key>& qkeys = qkey_map.get_QtKeys(native_key);
                for (Qt::Key qkey : qkeys){
                    const ControllerButton* button = button_lookup(qkey);
                    if (button != nullptr){
                        button->press(current);
                    }
                }
            }
        }
        ControllerState state;
        bool neutral = current.to_state(state);
//        cout << "neutral = " << neutral << endl;


        //  Send the command.
        WallClock now;
        do{
            now = current_time();
            try{
//                current.print();
                if (current == last && last_press + std::chrono::milliseconds(1000) > now){
//                    cout << "No state change." << endl;
                    break;
                }


                //  If state is neutral, just issue a stop.
                if (neutral){
                    if (try_stop_commands()){
                        last = VirtualControllerState();
                        last_neutral = true;
                        last_press = now;
                    }else{
                        next_wake = now + std::chrono::milliseconds(PABB_RETRANSMIT_DELAY_MILLIS);
                    }
                    break;
                }

                //  If the new state is different, set next interrupt so the new
                //  new command can replace the current one without gaps.
                if (!last_neutral && current != last && !try_next_interrupt()){
                    next_wake = now + std::chrono::milliseconds(PABB_RETRANSMIT_DELAY_MILLIS);
                    break;
                }

                //  Send the command.
                m_logger.log(
                    "VirtualController: (" + button_to_string(state.buttons) +
                    "), dpad(" + dpad_to_string(state.dpad) +
                    "), LJ(" + std::to_string(state.left_x) + "," + std::to_string(state.left_y) +
                    "), RJ(" + std::to_string(state.right_x) + "," + std::to_string(state.right_y) +
                    ")",
                    COLOR_DARKGREEN
                );
                std::string error = m_session.try_run<SwitchController>([=](SwitchController& controller){
                    controller.issue_controller_state(
                        nullptr,
                        state.buttons,
                        state.dpad,
                        state.left_x,
                        state.left_y,
                        state.right_x,
                        state.right_y,
                        255*8ms
                    );
                });
                if (!error.empty()){
                    next_wake = now + std::chrono::milliseconds(PABB_RETRANSMIT_DELAY_MILLIS);
                    break;
                }

                last = current;
                last_neutral = false;
                last_press = now;
            }catch (ProgramCancelledException&){
            }catch (ProgramFinishedException&){
            }catch (InvalidConnectionStateException&){
            }
        }while (false);


        //  Wait for next event.
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        if (m_stop.load(std::memory_order_acquire)){
            return;
        }

        //  Nothing held down.
        if (last_neutral && next_wake == WallClock::max()){
//            cout << "wait - long" << endl;
            m_cv.wait(lg);
        }else{
//            cout << "wait - short" << endl;
            m_cv.wait_until(lg, std::min(next_wake, now + std::chrono::milliseconds(1000)));
        }
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





