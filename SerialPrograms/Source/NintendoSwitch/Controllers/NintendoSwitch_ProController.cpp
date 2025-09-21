/*  Nintendo Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonTools/Async/InterruptableCommands.tpp"
#include "CommonTools/Async/SuperControlSession.tpp"
#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch_ProController.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{

//  Instantiate some template helper classes.
template class AsyncCommandSession<NintendoSwitch::ProController>;
template class SuperControlSession<NintendoSwitch::ProController>;

namespace NintendoSwitch{

using namespace std::chrono_literals;


const char ProController::NAME[] = "Nintendo Switch: Pro Controller";




class ProController::KeyboardManager final :
    public PokemonAutomation::KeyboardManager<ProControllerState, ProControllerDeltas>
{
public:
    KeyboardManager(Logger& logger, ProController& controller)
        : PokemonAutomation::KeyboardManager<ProControllerState, ProControllerDeltas>(logger, controller)
    {
        std::vector<std::shared_ptr<EditableTableRow>> mapping =
            ConsoleSettings::instance().KEYBOARD_MAPPINGS.PRO_CONTROLLER1.current_refs();
        for (const auto& deltas : mapping){
            const ProControllerKeyMapTableRow& row = static_cast<const ProControllerKeyMapTableRow&>(*deltas);
            m_mapping[(Qt::Key)(uint32_t)row.key] += row.snapshot();
        }
        start();
    }
    ~KeyboardManager(){
        stop();
    }
    virtual void send_state(const ControllerState& state) override{        
        const ProControllerState& switch_state = static_cast<const ProControllerState&>(state);

#if 0
        m_controller.logger().log(
            "VirtualController: (" + button_to_string(switch_state.buttons) +
            "), dpad(" + dpad_to_string(switch_state.dpad) +
            "), LJ(" + std::to_string(switch_state.left_x) + "," + std::to_string(switch_state.left_y) +
            "), RJ(" + std::to_string(switch_state.right_x) + "," + std::to_string(switch_state.right_y) +
            ")",
            COLOR_DARKGREEN
        );
#endif
        WriteSpinLock lg(m_lock);
        if (m_controller == nullptr){
            return;
        }
        Milliseconds ticksize = m_controller->ticksize();
        WallClock time_stamp = current_time();
        static_cast<ProController*>(m_controller)->issue_full_controller_state(
            nullptr,
            ticksize == Milliseconds::zero() ? 2000ms : ticksize * 255,
            switch_state.buttons,
            switch_state.dpad,
            switch_state.left_x,
            switch_state.left_y,
            switch_state.right_x,
            switch_state.right_y
        );

        report_keyboard_command_sent(time_stamp, switch_state);
    }


};



ProController::ProController(Logger& logger)
    : m_keyboard_manager(CONSTRUCT_TOKEN, logger, *this)
{

}
ProController::~ProController(){
}
void ProController::stop() noexcept{
    m_keyboard_manager->stop();
}


void ProController::keyboard_release_all(){
    m_keyboard_manager->clear_state();
}
void ProController::keyboard_press(const QKeyEvent& event){
    m_keyboard_manager->on_key_press(event);
}
void ProController::keyboard_release(const QKeyEvent& event){
    m_keyboard_manager->on_key_release(event);
}

void ProController::add_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener){
    m_keyboard_manager->add_listener(keyboard_listener);
}

void ProController::remove_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener){
    m_keyboard_manager->remove_listener(keyboard_listener);
}




}
}
