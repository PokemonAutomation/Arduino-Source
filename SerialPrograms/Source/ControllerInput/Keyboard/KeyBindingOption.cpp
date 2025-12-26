/*  Key Binding Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "KeyBindingOption.h"

namespace PokemonAutomation{



struct KeyboardHidBindingCell::Data{
    const KeyboardKey m_default;

    mutable SpinLock m_lock;
    KeyboardKey m_current;
    std::string m_current_text;

    Data(KeyboardKey default_value)
        : m_default(default_value)
        , m_current(default_value)
    {}
};



KeyboardHidBindingCell::~KeyboardHidBindingCell(){

}
KeyboardHidBindingCell::KeyboardHidBindingCell(LockMode lock_while_program_is_running)
    : ConfigOptionImpl<KeyboardHidBindingCell>(lock_while_program_is_running)
    , m_data(CONSTRUCT_TOKEN, (KeyboardKey)0)
{}


KeyboardHidBindingCell::operator KeyboardKey() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current;
}
KeyboardHidBindingCell::operator std::string() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current_text;
}
void KeyboardHidBindingCell::set(KeyboardKey key){
    {
        ReadSpinLock lg(m_data->m_lock);
        m_data->m_current = key;
    }
    report_value_changed(this);
}
void KeyboardHidBindingCell::set(std::string text){
    ReadSpinLock lg(m_data->m_lock);
    m_data->m_current_text = std::move(text);
}



void KeyboardHidBindingCell::load_json(const JsonValue& json){
    set((KeyboardKey)json.to_integer_default(0));
}
JsonValue KeyboardHidBindingCell::to_json() const{
    ReadSpinLock lg(m_data->m_lock);
    return (uint64_t)m_data->m_current;
}

void KeyboardHidBindingCell::restore_defaults(){
    set(m_data->m_default);
}

























}
