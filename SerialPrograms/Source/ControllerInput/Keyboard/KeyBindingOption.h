/*  Key Binding Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_KeyBindingOption_H
#define PokemonAutomation_Options_KeyBindingOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/ConfigOption.h"
#include "ControllerInput/Keyboard/KeyboardHidButtons.h"

namespace PokemonAutomation{




class KeyboardHidBindingCell : public ConfigOptionImpl<KeyboardHidBindingCell>{
public:
    ~KeyboardHidBindingCell();
    KeyboardHidBindingCell(LockMode lock_while_program_is_running);

    operator KeyboardKey() const;
    operator std::string() const;
    void set(KeyboardKey key);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;


public:
    //  UI Functions
    void set(std::string text);


private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
