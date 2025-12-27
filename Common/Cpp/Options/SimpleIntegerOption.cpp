/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <limits>
#include <atomic>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "SimpleIntegerOption.h"

namespace PokemonAutomation{



uint64_t SimpleIntegerCellBase::sanitize(uint64_t x){
    if (x > (uint64_t)std::numeric_limits<NativeType>::max()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Integer Overflow");
    }
    return x;
}





struct SimpleIntegerCellBase::Data{
    const NativeType m_min_value;
    const NativeType m_max_value;
    const NativeType m_default;
    std::atomic<NativeType> m_current;

    Data(
        NativeType min_value, NativeType max_value,
        NativeType default_value,
        NativeType value
    )
        : m_min_value(min_value)
        , m_max_value(max_value)
        , m_default(default_value)
        , m_current(value)
    {}
};




SimpleIntegerCellBase::~SimpleIntegerCellBase() = default;
SimpleIntegerCellBase::SimpleIntegerCellBase(const SimpleIntegerCellBase& x)
    : ConfigOptionImpl<SimpleIntegerCellBase>(x)
    , m_data(CONSTRUCT_TOKEN, x.min_value(), x.max_value(), x.default_value(), x.current_value())
{}
SimpleIntegerCellBase::SimpleIntegerCellBase(
    LockMode lock_while_running,
    NativeType min_value, NativeType max_value,
    NativeType default_value, NativeType current_value
)
    : ConfigOptionImpl<SimpleIntegerCellBase>(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, min_value, max_value, default_value, current_value)
{}

SimpleIntegerCellBase::SimpleIntegerCellBase(
    LockMode lock_while_running,
    NativeType default_value
)
    : ConfigOptionImpl<SimpleIntegerCellBase>(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        std::numeric_limits<NativeType>::min(),
        std::numeric_limits<NativeType>::max(),
        default_value,
        default_value
    )
{}
SimpleIntegerCellBase::SimpleIntegerCellBase(
    LockMode lock_while_running,
    NativeType default_value, NativeType min_value
)
    : ConfigOptionImpl<SimpleIntegerCellBase>(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        min_value,
        std::numeric_limits<NativeType>::max(),
        default_value,
        default_value
    )
{}
SimpleIntegerCellBase::SimpleIntegerCellBase(
    LockMode lock_while_running,
    NativeType default_value, NativeType min_value, NativeType max_value
)
    : ConfigOptionImpl<SimpleIntegerCellBase>(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, min_value, max_value, default_value, default_value)
{}

SimpleIntegerCellBase::NativeType SimpleIntegerCellBase::min_value() const{
    return m_data->m_min_value;
}
SimpleIntegerCellBase::NativeType SimpleIntegerCellBase::max_value() const{
    return m_data->m_max_value;
}
SimpleIntegerCellBase::NativeType SimpleIntegerCellBase::default_value() const{
    return m_data->m_default;
}
SimpleIntegerCellBase::NativeType SimpleIntegerCellBase::current_value() const{
    return m_data->m_current.load(std::memory_order_relaxed);
}
//SimpleIntegerCellBase::operator NativeType() const{
//    return m_data->m_current.load(std::memory_order_relaxed);
//}
std::string SimpleIntegerCellBase::set(NativeType x){
    std::string err = check_validity(x);
    if (!err.empty()){
        return err;
    }
    if (current_value() == x){
        return std::string();
    }
    if (x != m_data->m_current.exchange(x, std::memory_order_relaxed)){
        this->report_value_changed(this);
    }
    return std::string();
}
void SimpleIntegerCellBase::load_json(const JsonValue& json){
    NativeType value;
    if (json.read_integer(value, m_data->m_min_value, m_data->m_max_value)){
        set(value);
    }
}
JsonValue SimpleIntegerCellBase::to_json() const{
    return current_value();
}

std::string SimpleIntegerCellBase::check_validity(NativeType x) const{
    if (x < m_data->m_min_value){
        return "Value too small: min = " + std::to_string(m_data->m_min_value) + ", value = " + std::to_string(x);
    }
    if (x > m_data->m_max_value){
        return "Value too large: max = " + std::to_string(m_data->m_max_value) + ", value = " + std::to_string(x);
    }
    return std::string();
}
std::string SimpleIntegerCellBase::check_validity() const{
    return check_validity(current_value());
}
void SimpleIntegerCellBase::restore_defaults(){
    set(m_data->m_default);
}







SimpleIntegerOptionBase::SimpleIntegerOptionBase(
    std::string label,
    LockMode lock_while_running,
    NativeType min_value, NativeType max_value,
    NativeType default_value, NativeType current_value
)
    : ConfigOptionImpl<SimpleIntegerOptionBase, SimpleIntegerCellBase>(
        lock_while_running,
        min_value,
        max_value,
        default_value,
        current_value
    )
    , m_label(std::move(label))
{}
SimpleIntegerOptionBase::SimpleIntegerOptionBase(
    std::string label,
    LockMode lock_while_running,
    NativeType default_value
)
    : ConfigOptionImpl<SimpleIntegerOptionBase, SimpleIntegerCellBase>(
        lock_while_running,
        default_value
    )
    , m_label(std::move(label))
{}
SimpleIntegerOptionBase::SimpleIntegerOptionBase(
    std::string label,
    LockMode lock_while_running,
    NativeType default_value, NativeType min_value
)
    : ConfigOptionImpl<SimpleIntegerOptionBase, SimpleIntegerCellBase>(
        lock_while_running,
        default_value,
        min_value
    )
    , m_label(std::move(label))
{}
SimpleIntegerOptionBase::SimpleIntegerOptionBase(
    std::string label,
    LockMode lock_while_running,
    NativeType default_value, NativeType min_value, NativeType max_value
)
    : ConfigOptionImpl<SimpleIntegerOptionBase, SimpleIntegerCellBase>(
        lock_while_running,
        default_value,
        min_value,
        max_value
    )
    , m_label(std::move(label))
{}





}
