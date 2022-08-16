/*  Text Edit
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "TextEditOption.h"

namespace PokemonAutomation{



TextEditOption::TextEditOption(
    std::string label,
    std::string default_value,
    std::string placeholder_text
)
    : m_label(std::move(label))
    , m_default(std::move(default_value))
    , m_placeholder_text(std::move(placeholder_text))
    , m_current(m_default)
{}
#if 0
std::unique_ptr<ConfigOption> TextEditOption::clone() const{
   std::unique_ptr<TextEditOption> ret(new TextEditOption(m_label, m_default, m_placeholder_text));
   ret->m_current = m_current;
   return ret;
}
#endif

TextEditOption::operator const std::string&() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void TextEditOption::set(std::string x){
    {
        SpinLockGuard lg(m_lock);
        m_current = std::move(x);
    }
    push_update();
}


void TextEditOption::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    {
        SpinLockGuard lg(m_lock);
        m_current = *str;
    }
    push_update();
}
JsonValue TextEditOption::to_json() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void TextEditOption::restore_defaults(){
    {
        SpinLockGuard lg(m_lock);
        m_current = m_default;
    }
    push_update();
}











}
