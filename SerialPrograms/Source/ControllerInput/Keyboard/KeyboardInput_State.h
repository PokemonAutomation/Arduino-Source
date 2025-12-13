/*  Keyboard Input State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_Keyboard_KeyboardInput_State_H
#define PokemonAutomation_ControllerInput_Keyboard_KeyboardInput_State_H

#include <set>
#include "ControllerInput/ControllerInput.h"
#include "KeyboardHidButtons.h"

namespace PokemonAutomation{



class KeyboardInputState final : public ControllerInputState{
public:
    KeyboardInputState(KeyboardInputState&& x)
        : ControllerInputState(ControllerInputType::HID_Keyboard)
        , m_keys(std::move(x.m_keys))
    {}
    void operator=(KeyboardInputState&& x){
        if (this == &x){
            return;
        }
        m_keys = std::move(x.m_keys);
    }


public:
    KeyboardInputState()
        : ControllerInputState(ControllerInputType::HID_Keyboard)
    {}


    virtual void clear() override{
        m_keys.clear();
    }
    virtual bool is_neutral() const override{
        return m_keys.empty();
    }
    virtual bool operator==(const ControllerInputState& state) const override{
        const KeyboardInputState* x = dynamic_cast<const KeyboardInputState*>(&state);
        if (x == nullptr){
            return false;
        }
        if (m_keys.size() != x->m_keys.size()){
            return false;
        }
        for (auto a = m_keys.begin(), b = x->m_keys.begin(); a != m_keys.end(); ++a, ++b){
            KeyboardKey key_a = *a;
            KeyboardKey key_b = *b;
            if (key_a != key_b){
                return false;
            }
        }
        return true;
    }

    const std::set<KeyboardKey>& keys() const{
        return m_keys;
    }

    void add(KeyboardKey key){
        m_keys.insert(key);
    }


private:
    std::set<KeyboardKey> m_keys;
};






}
#endif
