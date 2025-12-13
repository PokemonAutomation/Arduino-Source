/*  HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonTools/Async/InterruptableCommands.tpp"
#include "CommonTools/Async/SuperControlSession.tpp"
#include "ControllerInput/ControllerInput.h"
#include "ControllerInput/Keyboard/KeyboardInput_State.h"
#include "ControllerInput/Keyboard/KeyboardInput_KeyMappings.h"
#include "Controllers/ControllerState.h"
#include "StandardHid_Keyboard.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

//  Instantiate some template helper classes.
template class AsyncCommandSession<StandardHid::Keyboard>;
template class SuperControlSession<StandardHid::Keyboard>;

namespace StandardHid{

using namespace std::chrono_literals;


const char Keyboard::NAME[] = "HID: Keyboard";




class KeyboardState : public ControllerState{
public:
    virtual void clear() override{
        keys.clear();
    }
    virtual bool operator==(const ControllerState& x) const override{
        if (typeid(*this) != typeid(x)){
            return false;
        }

        const KeyboardState& r = static_cast<const KeyboardState&>(x);
        if (keys.size() != r.keys.size()){
            return false;
        }

        auto iter0 = keys.begin();
        auto iter1 = r.keys.begin();
        for (; iter0 != keys.end(); ++iter0, ++iter1){
            if (*iter0 != *iter1){
                return false;
            }
        }

        return true;
    }
    virtual bool is_neutral() const override{
        return keys.empty();
    }

public:
    std::set<KeyboardKey> keys;
};







void log_qtkey(Logger& logger, const QtKeyMap::QtKey& qtkey){
    std::stringstream ss;
    ss << "Pressed key " << QTKEY_TO_STRING().at(qtkey.key);
    if (qtkey.keypad){
        ss << " (keypad)";
    }else{
        ss << " (main keyboard)";
    }

    const std::map<QtKeyMap::QtKey, KeyboardKey>& qwerty_hid_map = KEYID_TO_HID_QWERTY();
    if (auto iter = qwerty_hid_map.find(qtkey); iter != qwerty_hid_map.end()){
        ss << " mapped to QWERTY HID key " << KEYBOARDKEY_TO_STRING().at(iter->second);
    }else{
        ss << " has no QWERTY HID mapping";
    }

    const std::map<QtKeyMap::QtKey, KeyboardKey>& azerty_hid_map = KEYID_TO_HID_AZERTY();
    if (auto iter = azerty_hid_map.find(qtkey); iter != azerty_hid_map.end()){
        ss << " mapped to AZERTY HID key " << KEYBOARDKEY_TO_STRING().at(iter->second);
    }
    else {
        ss << " has no AZERTY HID mapping";
    }

    logger.log(ss.str(), COLOR_BLUE);
}






Keyboard::Keyboard(Logger& logger){

}
Keyboard::~Keyboard(){
}


void Keyboard::controller_input_state(const ControllerInputState& state){

    if (state.type() != ControllerInputType::HID_Keyboard){
        return;
    }

    const KeyboardInputState& lstate = static_cast<const KeyboardInputState&>(state);

    replace_on_next_command();

//    WallClock timestamp = current_time();

    issue_keys(
        nullptr,
        2000ms, 2000ms, 0ms,
        std::vector<KeyboardKey>(lstate.keys().begin(), lstate.keys().end())
    );

//    m_data->m_input_sniffer.report_keyboard_command_sent(timestamp, controller_state);
}






}
}
