/*  Standard Gamepad Input State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "GamepadInput_State.h"

namespace PokemonAutomation{

GamepadInputState::GamepadInputState()
    : ControllerInputState(ControllerInputType::StandardGamepad)
{}

GamepadInputState& GamepadInputState::operator=(const GamepadInputState& other){
    controller_state = other.controller_state;
    return *this;
}

void GamepadInputState::clear(){
    controller_state.clear();
}

bool GamepadInputState::is_neutral() const{
    return controller_state.is_neutral();
}

bool GamepadInputState::operator==(const ControllerInputState& state) const{
    const auto* other = dynamic_cast<const GamepadInputState*>(&state);
    if (other == nullptr){
        return false;
    }
    return controller_state == other->controller_state;
}

void GamepadInputGate::reset(){
    m_armed = false;
}

void GamepadInputGate::filter(GamepadInputState& state, bool has_listeners){
    if (!has_listeners){
        m_armed = false;
    }
    if (!m_armed){
        m_armed = has_listeners && state.is_neutral();
        state.clear();
    }
}

}
