/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <atomic>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "BooleanCheckBoxOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


struct BooleanCheckBoxCell::Data{
    const bool m_default;
    std::atomic<bool> m_current;

    Data(bool default_value, bool current_value)
        : m_default(default_value)
        , m_current(current_value)
    {}
};

BooleanCheckBoxCell::~BooleanCheckBoxCell() = default;
BooleanCheckBoxCell::BooleanCheckBoxCell(const BooleanCheckBoxCell& x)
    : ConfigOption(x)
    , m_data(CONSTRUCT_TOKEN, x.default_value(), x.current_value())
{}
BooleanCheckBoxCell::BooleanCheckBoxCell(
    LockMode lock_while_running,
    bool default_value, bool current_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, default_value, current_value)
{}
BooleanCheckBoxCell::BooleanCheckBoxCell(
    LockMode lock_while_running,
    bool default_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, default_value, default_value)
{}

bool BooleanCheckBoxCell::default_value() const{
    return m_data->m_default;
}
bool BooleanCheckBoxCell::current_value() const{
    return m_data->m_current.load(std::memory_order_relaxed);
}
BooleanCheckBoxCell::operator bool() const{
    return m_data->m_current.load(std::memory_order_relaxed);
}
void BooleanCheckBoxCell::operator=(bool x){
    if (x != m_data->m_current.exchange(x, std::memory_order_relaxed)){
        report_value_changed(this);
    }
}
void BooleanCheckBoxCell::load_json(const JsonValue& json){
    bool value;
    if (json.read_boolean(value)){
        *this = value;
    }
}
JsonValue BooleanCheckBoxCell::to_json() const{
    return (bool)*this;
}
void BooleanCheckBoxCell::restore_defaults(){
    *this = m_data->m_default;
}




BooleanCheckBoxOption::BooleanCheckBoxOption(
    std::string label,
    LockMode lock_while_running,
    bool default_value
)
    : BooleanCheckBoxCell(lock_while_running, default_value)
    , m_label(std::move(label))
{}
BooleanCheckBoxOption::BooleanCheckBoxOption(
    std::string label,
    LockMode lock_while_running,
    bool default_value, bool value
)
    : BooleanCheckBoxCell(lock_while_running, default_value, value)
    , m_label(std::move(label))
{}



}
