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
    LockMode lock_while_program_is_running,
    std::string default_value,
    std::string placeholder_text
)
    : ConfigOption(lock_while_program_is_running)
    , m_data(CONSTRUCT_TOKEN, is_password, std::move(default_value), std::move(placeholder_text))
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
const std::string StringCell::default_value() const{
    return m_data->m_default;
}

StringCell::operator std::string() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current;
}
void StringCell::set(std::string x){
    sanitize(x);
    {
        WriteSpinLock lg(m_data->m_lock);
        if (m_data->m_current == x){
            return;
        }
        m_data->m_current = std::move(x);
    }
    report_value_changed(this);
}

void StringCell::load_json(const JsonValue& json){
    const std::string* str = json.to_string();
    if (str == nullptr){
        return;
    }
    set(*str);
}
JsonValue StringCell::to_json() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current;
}

void StringCell::restore_defaults(){
    set(m_data->m_default);
}



StringOption::StringOption(
    bool is_password,
    std::string label,
    LockMode lock_while_program_is_running,
    std::string default_value,
    std::string placeholder_text
)
     : StringCell(is_password, lock_while_program_is_running, default_value, placeholder_text)
     , m_label(std::move(label))
{}







}
