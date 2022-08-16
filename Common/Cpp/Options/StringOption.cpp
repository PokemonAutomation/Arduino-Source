/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "StringOption.h"

namespace PokemonAutomation{



StringOption::StringOption(
    bool is_password,
    std::string label,
    std::string default_value,
    std::string placeholder_text
)
    : m_label(std::move(label))
    , m_is_password(is_password)
    , m_default(std::move(default_value))
    , m_placeholder_text(std::move(placeholder_text))
    , m_current(m_default)
{}
#if 0
std::unique_ptr<ConfigOption> StringOption::clone() const{
    std::unique_ptr<StringOption> ret(new StringOption(
        m_is_password,
        m_label,
        m_default,
        m_placeholder_text
    ));
    ret->m_current = m_current;
    return ret;
}
#endif

StringOption::operator std::string() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void StringOption::set(std::string x){
    {
        SpinLockGuard lg(m_lock);
        m_current = std::move(x);
    }
    push_update();
}

void StringOption::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr) {
        return;
    }
    {
        SpinLockGuard lg(m_lock);
        m_current = *str;
    }
    push_update();
}
JsonValue StringOption::to_json() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}

void StringOption::restore_defaults(){
    {
        SpinLockGuard lg(m_lock);
        m_current = m_default;
    }
    push_update();
}










}
