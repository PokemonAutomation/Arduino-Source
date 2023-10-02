/*  Integer Range Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#include <limits>
#include <atomic>
#include "Common/Cpp/Exceptions.h"
#include "IntegerRangeOption.h"

namespace PokemonAutomation{



template <typename Type>
struct IntegerRangeCell<Type>::Data{
    const Type m_lo_min_value;
    const Type m_lo_max_value;
    const Type m_hi_min_value;
    const Type m_hi_max_value;
    const Type m_lo_default;
    const Type m_hi_default;
    std::atomic<Type> m_lo_current;
    std::atomic<Type> m_hi_current;

    Data(
        Type lo_min_value, Type lo_max_value, Type lo_default_value, Type lo_current_value,
        Type hi_min_value, Type hi_max_value, Type hi_default_value, Type hi_current_value
    )
        : m_lo_min_value(lo_min_value), m_lo_max_value(lo_max_value)
        , m_hi_min_value(hi_min_value), m_hi_max_value(hi_max_value)
        , m_lo_default(lo_default_value)
        , m_hi_default(hi_default_value)
        , m_lo_current(lo_current_value)
        , m_hi_current(hi_current_value)
    {
        if (lo_min_value > lo_max_value){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Limits are incompatible.");
        }
        if (hi_min_value > hi_max_value){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Limits are incompatible.");
        }
        if (!(lo_min_value <= lo_default_value && lo_default_value <= lo_max_value)){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Low default is out of range.");
        }
        if (!(hi_min_value <= hi_default_value && hi_default_value <= hi_max_value)){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "High default is out of range.");
        }
        if (lo_default_value > hi_default_value){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Default values are incompatible.");
        }
        if (lo_current_value > hi_current_value){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Current values are incompatible.");
        }
    }
};



template <typename Type>
IntegerRangeCell<Type>::~IntegerRangeCell() = default;
template <typename Type>
IntegerRangeCell<Type>::IntegerRangeCell(const IntegerRangeCell& x)
    : ConfigOption(x)
    , m_data(
        CONSTRUCT_TOKEN,
        x.lo_min_value(), x.lo_max_value(), x.lo_default_value(), x.lo_current_value(),
        x.hi_min_value(), x.hi_max_value(), x.hi_default_value(), x.hi_current_value()
    )
{}
template <typename Type>
IntegerRangeCell<Type>::IntegerRangeCell(
    LockWhileRunning lock_while_running,
    Type lo_min_value, Type lo_max_value, Type lo_default_value, Type lo_current_value,
    Type hi_min_value, Type hi_max_value, Type hi_default_value, Type hi_current_value
)
    : ConfigOption(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        lo_min_value, lo_max_value, lo_default_value, lo_current_value,
        hi_min_value, hi_max_value, hi_default_value, hi_current_value
    )
{}

template <typename Type>
Type IntegerRangeCell<Type>::lo_min_value() const{
    return m_data->m_lo_min_value;
}
template <typename Type>
Type IntegerRangeCell<Type>::lo_max_value() const{
    return m_data->m_lo_max_value;
}
template <typename Type>
Type IntegerRangeCell<Type>::lo_default_value() const{
    return m_data->m_lo_default;
}
template <typename Type>
Type IntegerRangeCell<Type>::lo_current_value() const{
    return m_data->m_lo_current.load(std::memory_order_relaxed);
}

template <typename Type>
Type IntegerRangeCell<Type>::hi_min_value() const{
    return m_data->m_hi_min_value;

}
template <typename Type>
Type IntegerRangeCell<Type>::hi_max_value() const{
    return m_data->m_hi_max_value;
}
template <typename Type>
Type IntegerRangeCell<Type>::hi_default_value() const{
    return m_data->m_hi_default;
}
template <typename Type>
Type IntegerRangeCell<Type>::hi_current_value() const{
    return m_data->m_hi_current.load(std::memory_order_relaxed);
}

template <typename Type>
void IntegerRangeCell<Type>::set_lo(Type x){
    x = std::max(x, lo_min_value());
    x = std::min(x, lo_max_value());
    x = std::min(x, hi_current_value());
}
template <typename Type>
void IntegerRangeCell<Type>::set_hi(Type x){
    x = std::max(x, hi_min_value());
    x = std::min(x, hi_max_value());
    x = std::max(x, lo_current_value());

}







template class IntegerRangeCell<uint8_t>;



}

