/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <atomic>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "FloatingPointOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


struct FloatingPointCell::Data{
    const double m_min_value;
    const double m_max_value;
    const double m_default;
    std::atomic<double> m_current;

    Data(
        double min_value, double max_value,
        double default_value, double current_value
    )
        : m_min_value(min_value)
        , m_max_value(max_value)
        , m_default(default_value)
        , m_current(current_value)
    {}
};



FloatingPointCell::~FloatingPointCell() = default;
FloatingPointCell::FloatingPointCell(const FloatingPointCell& x)
    : ConfigOption(x)
    , m_data(CONSTRUCT_TOKEN, x.min_value(), x.max_value(), x.default_value(), x)
{}
FloatingPointCell::FloatingPointCell(
    LockMode lock_while_running,
    double min_value, double max_value,
    double default_value, double current_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, min_value, max_value, default_value, current_value)
{}


FloatingPointCell::FloatingPointCell(
    LockMode lock_while_running,
    double default_value,
    double min_value,
    double max_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, min_value, max_value, default_value, default_value)
{}


double FloatingPointCell::min_value() const{
    return m_data->m_min_value;
}
double FloatingPointCell::max_value() const{
    return m_data->m_max_value;
}
double FloatingPointCell::default_value() const{
    return m_data->m_default;
}

FloatingPointCell::operator double() const{
    return m_data->m_current.load(std::memory_order_relaxed);
}


std::string FloatingPointCell::set(double x){
    std::string err = check_validity(x);
    if (!err.empty()){
        return err;
    }
    if (x != m_data->m_current.exchange(x, std::memory_order_relaxed)){
        report_value_changed(this);
    }
    return err;
}

void FloatingPointCell::load_json(const JsonValue& json){
    double value;
    if (!json.read_float(value)){
        return;
    }
    Data& data = *m_data;
    value = std::max(value, data.m_min_value);
    value = std::min(value, data.m_max_value);
    if (std::isnan(value)){
        value = data.m_default;
    }
    set(value);
}
JsonValue FloatingPointCell::to_json() const{
    return (double)*this;
}

std::string FloatingPointCell::check_validity(double x) const{
    const Data& data = *m_data;
    if (x < data.m_min_value){
        std::ostringstream ss;
        return "Value too small: min = " + tostr_default(data.m_min_value) + ", value = " + tostr_default(x);
    }
    if (x > data.m_max_value){
        std::ostringstream ss;
        return "Value too large: max = " + tostr_default(data.m_max_value) + ", value = " + tostr_default(x);
    }
    if (std::isnan(x)){
        return "Value is NaN.";
    }
    return std::string();
}
std::string FloatingPointCell::check_validity() const{
    return check_validity(*this);
}
void FloatingPointCell::restore_defaults(){
    set(m_data->m_default);
}





FloatingPointOption::FloatingPointOption(
    std::string label,
    LockMode lock_while_running,
    double default_value,
    double min_value,
    double max_value
)
    : FloatingPointCell(lock_while_running, default_value, min_value, max_value)
    , m_label(std::move(label))
{}




}
