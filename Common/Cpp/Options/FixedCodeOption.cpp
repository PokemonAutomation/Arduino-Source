/*  Fixed Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/CodeValidator.h"
#include "FixedCodeOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


struct FixedCodeOption::Data{
    std::string m_label;
    const size_t m_digits;
    const std::string m_default;

    mutable SpinLock m_lock;
    std::string m_current;

    Data(
        std::string label,
        size_t digits,
        std::string default_value
    )
        : m_label(std::move(label))
        , m_digits(digits)
        , m_default(default_value)
        , m_current(std::move(default_value))
    {}
};


FixedCodeOption::~FixedCodeOption() = default;
FixedCodeOption::FixedCodeOption(
    std::string label,
    size_t digits,
    std::string default_value
)
    : m_data(CONSTRUCT_TOKEN, std::move(label), digits, std::move(default_value))
{}
#if 0
std::unique_ptr<ConfigOption> FixedCodeOption::clone() const{
    std::unique_ptr<FixedCodeOption> ret(new FixedCodeOption(m_label, m_digits, m_default));
    ret->m_current = m_current;
    return ret;
}
#endif

const std::string& FixedCodeOption::label() const{
    return m_data->m_label;
}
size_t FixedCodeOption::digits() const{
    return m_data->m_digits;
}

FixedCodeOption::operator const std::string&() const{
    SpinLockGuard lg(m_data->m_lock);
    return m_data->m_current;
}
const std::string& FixedCodeOption::get() const{
    SpinLockGuard lg(m_data->m_lock);
    return m_data->m_current;
}
std::string FixedCodeOption::set(std::string x){
    std::string error = check_validity(x);
    if (!error.empty()){
        return error;
    }
    {
        SpinLockGuard lg(m_data->m_lock);
        m_data->m_current = std::move(x);
    }
    push_update();
    return std::string();
}

void FixedCodeOption::load_json(const JsonValue& json){
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
JsonValue FixedCodeOption::to_json() const{
    SpinLockGuard lg(m_data->m_lock);
    return m_data->m_current;
}

void FixedCodeOption::to_str(uint8_t* code) const{
    SpinLockGuard lg(m_data->m_lock);
    std::string qstr = sanitize_code(8, m_data->m_current);
    for (int c = 0; c < 8; c++){
        code[c] = qstr[c] - '0';
    }
}

std::string FixedCodeOption::check_validity() const{
    SpinLockGuard lg(m_data->m_lock);
    return check_validity(m_data->m_current);
}
std::string FixedCodeOption::check_validity(const std::string& x) const{
    return validate_code(m_data->m_digits, x) ? std::string() : "Code is invalid.";
}
void FixedCodeOption::restore_defaults(){
    {
        SpinLockGuard lg(m_data->m_lock);
        m_data->m_current = m_data->m_default;
    }
    push_update();
}






}
