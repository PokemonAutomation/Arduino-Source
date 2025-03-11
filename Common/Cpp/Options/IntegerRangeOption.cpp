/*  Integer Range Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This option is thread-safe.
 *
 */

#include <limits>
#include <atomic>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "IntegerRangeOption.h"

#include "Common/Qt/Options/IntegerRangeWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



template <typename Type>
ConfigWidget* IntegerRangeCell<Type>::make_QtWidget(QWidget& parent){
    return new IntegerRangeCellWidget<Type>(parent, *this);
}



template <typename Type>
struct IntegerRangeCell<Type>::Data{
    const Type m_lo_min_value;
    const Type m_lo_max_value;
    const Type m_hi_min_value;
    const Type m_hi_max_value;
    const Type m_lo_default;
    const Type m_hi_default;

    mutable SpinLock m_lock;
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
        if (lo_min_value > lo_max_value ||
            hi_min_value > hi_max_value ||
            lo_min_value > hi_min_value ||
            lo_max_value > hi_max_value ||
            lo_max_value < hi_min_value
        ){
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
    LockMode lock_while_running,
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
Type IntegerRangeCell<Type>::lo_current_value() const{
    return m_data->m_lo_current.load(std::memory_order_relaxed);
}
template <typename Type>
Type IntegerRangeCell<Type>::hi_current_value() const{
    return m_data->m_hi_current.load(std::memory_order_relaxed);
}
template <typename Type>
void IntegerRangeCell<Type>::current_values(Type& lo, Type& hi) const{
    ReadSpinLock lg(m_data->m_lock);
    lo = lo_current_value();
    hi = hi_current_value();
}

template <typename Type>
void IntegerRangeCell<Type>::set_lo(Type lo){
    lo = std::max(lo, lo_min_value());
    lo = std::min(lo, lo_max_value());

    Type current_lo;
    {
        WriteSpinLock lg(m_data->m_lock);
        current_lo = lo_current_value();
        if (hi_current_value() < lo){
            m_data->m_hi_current.store(lo, std::memory_order_relaxed);
        }
        if (current_lo != lo){
            m_data->m_lo_current.store(lo, std::memory_order_relaxed);
        }
    }
    //  It is impossible for hi to change without also changing lo. So we only
    //  need to check for lo. If this condition fails, we were already in a bad
    //  state to begin with.
    if (current_lo != lo){
        report_value_changed(this);
    }
}
template <typename Type>
void IntegerRangeCell<Type>::set_hi(Type hi){
    hi = std::max(hi, hi_min_value());
    hi = std::min(hi, hi_max_value());

    Type current_hi;
    {
        WriteSpinLock lg(m_data->m_lock);
        current_hi = hi_current_value();
        if (lo_current_value() > hi){
            m_data->m_lo_current.store(hi, std::memory_order_relaxed);
        }
        if (current_hi != hi){
            m_data->m_hi_current.store(hi, std::memory_order_relaxed);
        }
    }
    //  It is impossible for lo to change without also changing hi. So we only
    //  need to check for hi. If this condition fails, we were already in a bad
    //  state to begin with.
    if (current_hi != hi){
        report_value_changed(this);
    }
}
template <typename Type>
void IntegerRangeCell<Type>::set(Type lo, Type hi){
    lo = std::max(lo, lo_min_value());
    lo = std::min(lo, lo_max_value());
    hi = std::max(hi, hi_min_value());
    hi = std::min(hi, hi_max_value());
    hi = std::max(hi, lo);

    Type current_lo, current_hi;
    {
        WriteSpinLock lg(m_data->m_lock);
        current_lo = lo_current_value();
        current_hi = hi_current_value();
        if (current_lo != lo){
            m_data->m_lo_current.store(lo, std::memory_order_relaxed);
        }
        if (current_hi != hi){
            m_data->m_hi_current.store(hi, std::memory_order_relaxed);
        }
    }
    if (current_lo != lo || current_hi != hi){
        report_value_changed(this);
    }
}

template <typename Type>
void IntegerRangeCell<Type>::set(const IntegerRangeCell& option){
    Type lo, hi;
    option.current_values(lo, hi);
    set(lo, hi);
    current_values(lo, hi);
}

template <typename Type>
void IntegerRangeCell<Type>::load_json(const JsonValue& json){
    const JsonArray* array = json.to_array();
    if (array == nullptr || array->size() != 2){
        return;
    }
    set(
        (Type)(*array)[0].to_integer_default(m_data->m_lo_default),
        (Type)(*array)[1].to_integer_default(m_data->m_hi_default)
    );
}
template <typename Type>
JsonValue IntegerRangeCell<Type>::to_json() const{
    Type lo, hi;
    current_values(lo, hi);
    JsonArray array;
    array.push_back(lo);
    array.push_back(hi);
    return array;
}
template <typename Type>
void IntegerRangeCell<Type>::restore_defaults(){
    set(m_data->m_lo_default, m_data->m_hi_default);
}







template class IntegerRangeCell<uint8_t>;



}

