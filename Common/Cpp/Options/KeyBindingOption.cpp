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



struct KeyBindingCell::Data{
    const uint32_t m_default;

    mutable SpinLock m_lock;
    uint32_t m_current;
    std::string m_current_text;

    Data(uint32_t default_value)
        : m_default(default_value)
        , m_current(default_value)
    {}
};



KeyBindingCell::~KeyBindingCell(){

}
KeyBindingCell::KeyBindingCell(LockMode lock_while_program_is_running)
    : ConfigOption(lock_while_program_is_running)
    , m_data(CONSTRUCT_TOKEN, 0)
{}


KeyBindingCell::operator uint32_t() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current;
}
KeyBindingCell::operator std::string() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current_text;
}
void KeyBindingCell::set(uint32_t key){
    {
        ReadSpinLock lg(m_data->m_lock);
        m_data->m_current = key;
    }
    report_value_changed(this);
}
void KeyBindingCell::set(std::string text){
    ReadSpinLock lg(m_data->m_lock);
    m_data->m_current_text = std::move(text);
}



void KeyBindingCell::load_json(const JsonValue& json){
    set((uint32_t)json.to_integer_default(0));
}
JsonValue KeyBindingCell::to_json() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current;
}

void KeyBindingCell::restore_defaults(){
    set(m_data->m_default);
}









}
