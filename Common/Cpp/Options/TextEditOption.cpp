/*  Text Edit
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "TextEditOption.h"

namespace PokemonAutomation{


struct TextEditOption::Data{
    const std::string m_label;
    const std::string m_default;
    const std::string m_placeholder_text;

    mutable SpinLock m_lock;
    std::string m_current;

    Data(
        std::string label,
        std::string default_value,
        std::string placeholder_text
    )
        : m_label(std::move(label))
        , m_default(std::move(default_value))
        , m_placeholder_text(std::move(placeholder_text))
        , m_current(m_default)
    {}
};


TextEditOption::~TextEditOption() = default;
TextEditOption::TextEditOption(
    std::string label,
    std::string default_value,
    std::string placeholder_text
)
    : m_data(CONSTRUCT_TOKEN, std::move(label), std::move(default_value), std::move(placeholder_text))
{}
#if 0
std::unique_ptr<ConfigOption> TextEditOption::clone() const{
   std::unique_ptr<TextEditOption> ret(new TextEditOption(m_label, m_default, m_placeholder_text));
   ret->m_current = m_current;
   return ret;
}
#endif

const std::string& TextEditOption::label() const{
    return m_data->m_label;
}
const std::string& TextEditOption::placeholder_text() const{
    return m_data->m_placeholder_text;
}

TextEditOption::operator std::string() const{
    SpinLockGuard lg(m_data->m_lock);
    return m_data->m_current;
}
void TextEditOption::set(std::string x){
    {
        SpinLockGuard lg(m_data->m_lock);
        m_data->m_current = std::move(x);
    }
    push_update();
}


void TextEditOption::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    {
        SpinLockGuard lg(m_data->m_lock);
        m_data->m_current = *str;
    }
    push_update();
}
JsonValue TextEditOption::to_json() const{
    SpinLockGuard lg(m_data->m_lock);
    return m_data->m_current;
}
void TextEditOption::restore_defaults(){
    {
        SpinLockGuard lg(m_data->m_lock);
        m_data->m_current = m_data->m_default;
    }
    push_update();
}











}
