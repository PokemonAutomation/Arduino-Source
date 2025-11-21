/*  HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Options/KeyboardLayoutOption.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonTools/Async/InterruptableCommands.tpp"
#include "CommonTools/Async/SuperControlSession.tpp"
#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "StandardHid_Keyboard_KeyMappings.h"
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



const std::map<QtKeyMap::QtKey, KeyboardKey>& get_keyid_to_hid_map(){
    KeyboardLayout layout = *GlobalSettings::instance().KEYBOARD_CONTROLS_LAYOUT;
    switch (layout){
    case KeyboardLayout::QWERTY:
        return KEYID_TO_HID_QWERTY();
    case KeyboardLayout::AZERTY:
        return KEYID_TO_HID_AZERTY();
    default:
        throw InternalProgramError(
            nullptr,
            PA_CURRENT_FUNCTION,
            "Invalid KeyboardLayout Enum: " + std::to_string((int)layout)
        );
    }
}


class Keyboard::KeyboardManager final : public PokemonAutomation::KeyboardInputController{
public:
    KeyboardManager(Logger& logger, Keyboard& controller)
        : KeyboardInputController(logger, true)
        , m_controller(&controller)
    {
        KeyboardInputController::start();
    }
    void stop() noexcept{
        {
            WriteSpinLock lg(m_lock);
            if (m_controller == nullptr){
                return;
            }
            m_controller = nullptr;
        }
        KeyboardInputController::stop();
    }

    virtual std::unique_ptr<ControllerState> make_state() const override{
        return std::make_unique<KeyboardState>();
    }
    virtual void update_state(ControllerState& state, const std::set<uint32_t>& pressed_keys) override{
        const QtKeyMap& qkey_map = QtKeyMap::instance();

        const std::map<QtKeyMap::QtKey, KeyboardKey>& hid_map = get_keyid_to_hid_map();

        KeyboardState& local_state = static_cast<KeyboardState&>(state);
        local_state.clear();

        for (uint32_t native_key : pressed_keys){
            std::set<QtKeyMap::QtKey> qkeys = qkey_map.get_QtKeys(native_key);
            for (QtKeyMap::QtKey qkey : qkeys){
                auto iter = hid_map.find(qkey);
                if (iter != hid_map.end()){
                    local_state.keys.insert(iter->second);
                }
//                log_qtkey(m_logger, qkey);
            }
        }
    }
    virtual void cancel_all_commands() override{
        WriteSpinLock lg(m_lock);
        if (m_controller == nullptr){
            return;
        }
        m_controller->cancel_all_commands();
    }
    virtual void replace_on_next_command() override{
        WriteSpinLock lg(m_lock);
        if (m_controller == nullptr){
            return;
        }
        m_controller->replace_on_next_command();
    }
    virtual void send_state(const ControllerState& state) override{
        const KeyboardState& local_state = static_cast<const KeyboardState&>(state);
        WriteSpinLock lg(m_lock);
        if (m_controller == nullptr){
            return;
        }
        m_controller->issue_keys(
            nullptr,
            2000ms, 2000ms, 0ms,
            std::vector<KeyboardKey>(local_state.keys.begin(), local_state.keys.end())
        );
    }


protected:
    SpinLock m_lock;
    Keyboard* m_controller;
};








Keyboard::Keyboard(Logger& logger)
    : m_keyboard_manager(CONSTRUCT_TOKEN, logger, *this)
{

}
Keyboard::~Keyboard(){
    stop();
}
void Keyboard::stop() noexcept{
    m_keyboard_manager->stop();
}


void Keyboard::keyboard_release_all(){
    m_keyboard_manager->clear_state();
}
void Keyboard::keyboard_press(const QKeyEvent& event){
    m_keyboard_manager->on_key_press(event);
}
void Keyboard::keyboard_release(const QKeyEvent& event){
    m_keyboard_manager->on_key_release(event);
}






}
}
