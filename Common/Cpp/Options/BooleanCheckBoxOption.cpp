/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "BooleanCheckBoxOption.h"

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
BooleanCheckBoxCell::BooleanCheckBoxCell(bool default_value, bool current_value)
    : m_data(CONSTRUCT_TOKEN, default_value, current_value)
{}
BooleanCheckBoxCell::BooleanCheckBoxCell(bool default_value)
    : m_data(CONSTRUCT_TOKEN, default_value, default_value)
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
    m_data->m_current.store(x, std::memory_order_relaxed);
    push_update();
}
void BooleanCheckBoxCell::load_json(const JsonValue& json){
    bool value;
    if (json.read_boolean(value)){
        *this = value;
        push_update();
    }
}
JsonValue BooleanCheckBoxCell::to_json() const{
    return (bool)*this;
}
void BooleanCheckBoxCell::restore_defaults(){
    *this = m_data->m_default;
    push_update();
}




BooleanCheckBoxOption::BooleanCheckBoxOption(std::string label, bool default_value)
    : BooleanCheckBoxCell(default_value)
    , m_label(std::move(label))
{}
BooleanCheckBoxOption::BooleanCheckBoxOption(std::string label, bool default_value, bool value)
    : BooleanCheckBoxCell(default_value, value)
    , m_label(std::move(label))
{}



}
