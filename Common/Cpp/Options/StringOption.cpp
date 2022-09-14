/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "StringOption.h"

namespace PokemonAutomation{


struct StringCell::Data{
    const bool m_is_password;
    const std::string m_default;
    const std::string m_placeholder_text;

    mutable SpinLock m_lock;
    std::string m_current;

    Data(
        bool is_password,
        std::string default_value,
        std::string placeholder_text
    )
        : m_is_password(is_password)
        , m_default(std::move(default_value))
        , m_placeholder_text(std::move(placeholder_text))
        , m_current(m_default)
    {}
};


StringCell::~StringCell() = default;
StringCell::StringCell(
    bool is_password,
    std::string default_value,
    std::string placeholder_text
)
    : m_data(CONSTRUCT_TOKEN, is_password, std::move(default_value), std::move(placeholder_text))
{}
#if 0
std::unique_ptr<ConfigOption> StringCell::clone() const{
    std::unique_ptr<StringCell> ret(new StringCell(
        m_is_password,
        m_label,
        m_default,
        m_placeholder_text
    ));
    ret->m_current = m_current;
    return ret;
}
#endif

bool StringCell::is_password() const{
    return m_data->m_is_password;
}
const std::string& StringCell::placeholder_text() const{
    return m_data->m_placeholder_text;
}

StringCell::operator std::string() const{
    SpinLockGuard lg(m_data->m_lock);
    return m_data->m_current;
}
void StringCell::set(std::string x){
    {
        SpinLockGuard lg(m_data->m_lock);
        m_data->m_current = std::move(x);
    }
    push_update();
}

void StringCell::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr) {
        return;
    }
    {
        SpinLockGuard lg(m_data->m_lock);
        m_data->m_current = *str;
    }
    push_update();
}
JsonValue StringCell::to_json() const{
    SpinLockGuard lg(m_data->m_lock);
    return m_data->m_current;
}

void StringCell::restore_defaults(){
    {
        SpinLockGuard lg(m_data->m_lock);
        m_data->m_current = m_data->m_default;
    }
    push_update();
}



StringOption::StringOption(
    bool is_password,
    std::string label,
    std::string default_value,
    std::string placeholder_text
)
     : StringCell(is_password, default_value, placeholder_text)
     , m_label(std::move(label))
{}







}
