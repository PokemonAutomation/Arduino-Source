/*  Nintendo Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QKeyEvent>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonTools/Async/InterruptableCommands.tpp"
#include "CommonTools/Async/SuperControlSession.tpp"
#include "Controllers/KeyboardInput/GlobalQtKeyMap.h"
#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_ProController.h"
#include "NintendoSwitch_VirtualControllerState.h"

namespace PokemonAutomation{

//  Instantiate some template helper classes.
template class AsyncCommandSession<NintendoSwitch::ProController>;
template class SuperControlSession<NintendoSwitch::ProController>;

namespace NintendoSwitch{


using namespace std::chrono_literals;




class ProController::KeyboardManager : public KeyboardInputController{
public:
    KeyboardManager(ProController& controller)
        : KeyboardInputController(true)
        , m_controller(controller)
    {
        std::vector<std::shared_ptr<EditableTableRow>> mapping =
            ConsoleSettings::instance().KEYBOARD_MAPPINGS.TABLE.current_refs();
        for (const auto& deltas : mapping){
            const KeyMapTableRow& row = static_cast<const KeyMapTableRow&>(*deltas);
            m_mapping[(Qt::Key)(uint32_t)row.key] += row.snapshot();
        }
        start();
    }
    ~KeyboardManager(){
        stop();
    }

    virtual std::unique_ptr<ControllerState> make_state() const override{
        return std::make_unique<SwitchControllerState>();
    }
    virtual void update_state(ControllerState& state, const std::set<uint32_t>& pressed_keys) override{
        ControllerDeltas deltas;
        const QtKeyMap& qkey_map = QtKeyMap::instance();
        for (uint32_t native_key : pressed_keys){
            const std::set<Qt::Key>& qkeys = qkey_map.get_QtKeys(native_key);
            for (Qt::Key qkey : qkeys){
                auto iter = m_mapping.find(qkey);
                if (iter != m_mapping.end()){
                    deltas += iter->second;
                    break;
                }
            }
        }
        deltas.to_state(static_cast<SwitchControllerState&>(state));
    }
    virtual void cancel_all_commands() override{
        m_controller.cancel_all_commands();
    }
    virtual void replace_on_next_command() override{
        m_controller.replace_on_next_command();
    }
    virtual void send_state(const ControllerState& state) override{
        const SwitchControllerState& switch_state = static_cast<const SwitchControllerState&>(state);
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
        m_controller.issue_full_controller_state(
            nullptr,
            switch_state.buttons,
            switch_state.dpad,
            switch_state.left_x,
            switch_state.left_y,
            switch_state.right_x,
            switch_state.right_y,
            255 * 8ms
        );
    }


private:
    ProController& m_controller;
    std::map<Qt::Key, ControllerDeltas> m_mapping;
};



ProController::~ProController(){

}
ProController::ProController(Milliseconds timing_variation)
    : m_timing_variation(timing_variation)
    , m_keyboard_manager(CONSTRUCT_TOKEN, *this)
{

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







}
}
