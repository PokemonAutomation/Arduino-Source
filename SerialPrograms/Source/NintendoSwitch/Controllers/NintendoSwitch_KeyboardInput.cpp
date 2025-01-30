/*  Virtual Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Controllers/KeyboardInput/GlobalQtKeyMap.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "NintendoSwitch_KeyboardInput.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;





VirtualController::VirtualController(
    ControllerSession& session,
    bool allow_commands_while_running
)
    : KeyboardInputController(allow_commands_while_running)
    , m_session(session)
{
    std::vector<std::shared_ptr<EditableTableRow>> mapping =
        ConsoleSettings::instance().KEYBOARD_MAPPINGS.TABLE.current_refs();
    for (const auto& deltas : mapping){
        const KeyMapTableRow& row = static_cast<const KeyMapTableRow&>(*deltas);
        m_mapping[(Qt::Key)(uint32_t)row.key] += row.snapshot();
    }

    this->start();
}
VirtualController::~VirtualController(){
    this->stop();
}


std::unique_ptr<ControllerState> VirtualController::make_state() const{
    return std::make_unique<SwitchControllerState>();
}
void VirtualController::update_state(ControllerState& state, const std::set<uint32_t>& pressed_keys){
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
bool VirtualController::try_send_state(const ControllerState& state){
    return state.send_to_controller(m_session);
}







}
}





