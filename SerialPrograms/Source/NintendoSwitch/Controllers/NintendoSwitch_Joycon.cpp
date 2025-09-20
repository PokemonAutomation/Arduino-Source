/*  Nintendo Switch Joycon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonTools/Async/InterruptableCommands.tpp"
#include "CommonTools/Async/SuperControlSession.tpp"
#include "Controllers/ControllerTypes.h"
#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch_Joycon.h"

namespace PokemonAutomation{

//  Instantiate some template helper classes.
template class AsyncCommandSession<NintendoSwitch::JoyconController>;
template class SuperControlSession<NintendoSwitch::JoyconController>;

namespace NintendoSwitch{

using namespace std::chrono_literals;


const char JoyconController::NAME[] = "Nintendo Switch: Joycon";
const char LeftJoycon::NAME[] = "Nintendo Switch: Left Joycon";
const char RightJoycon::NAME[] = "Nintendo Switch: Right Joycon";



class JoyconController::KeyboardManager final :
    public PokemonAutomation::KeyboardManager<JoyconState, JoyconDeltas>
{
public:
    KeyboardManager(Logger& logger, JoyconController& controller, ControllerType controller_type)
        : PokemonAutomation::KeyboardManager<JoyconState, JoyconDeltas>(logger, controller)
    {
        std::vector<std::shared_ptr<EditableTableRow>> mapping;
        switch (controller_type){
        case ControllerType::NintendoSwitch_LeftJoycon:
            mapping = ConsoleSettings::instance().KEYBOARD_MAPPINGS.LEFT_JOYCON0.current_refs();
            break;
        case ControllerType::NintendoSwitch_RightJoycon:
            mapping = ConsoleSettings::instance().KEYBOARD_MAPPINGS.RIGHT_JOYCON0.current_refs();
            break;
        default:;
        }
        for (const auto& deltas : mapping){
            const JoyconKeyMapTableRow& row = static_cast<const JoyconKeyMapTableRow&>(*deltas);
            m_mapping[(Qt::Key)(uint32_t)row.key] += row.snapshot();
        }
        start();
    }
    ~KeyboardManager(){
        stop();
    }
    virtual void send_state(const ControllerState& state) override{
        
        const JoyconState& switch_state = static_cast<const JoyconState&>(state);
#if 0
        m_controller->logger().log(
            "VirtualController: (" + button_to_string(switch_state.buttons) +
            "), LJ(" + std::to_string(switch_state.joystick_x) + "," + std::to_string(switch_state.joystick_y) +
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
        static_cast<JoyconController*>(m_controller)->issue_full_controller_state(
            nullptr,
            switch_state.buttons,
            switch_state.joystick_x,
            switch_state.joystick_y,
            ticksize == Milliseconds::zero() ? 2000ms : ticksize * 255
        );
        report_keyboard_command_sent(time_stamp, switch_state);
    }
};



JoyconController::JoyconController(Logger& logger, ControllerType controller_type)
    : m_keyboard_manager(CONSTRUCT_TOKEN, logger, *this, controller_type)
{

}
JoyconController::~JoyconController(){

}
void JoyconController::stop() noexcept{
    m_keyboard_manager->stop();
}


void JoyconController::keyboard_release_all(){
    m_keyboard_manager->clear_state();
}
void JoyconController::keyboard_press(const QKeyEvent& event){
    m_keyboard_manager->on_key_press(event);
}
void JoyconController::keyboard_release(const QKeyEvent& event){
    m_keyboard_manager->on_key_release(event);
}

void JoyconController::add_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener){
    m_keyboard_manager->add_listener(keyboard_listener);
}

void JoyconController::remove_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener){
    m_keyboard_manager->remove_listener(keyboard_listener);
}





}
}
