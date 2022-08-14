/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "FloatingPointOption.h"

namespace PokemonAutomation{



FloatingPointOption::FloatingPointOption(
    std::string label,
    double default_value,
    double min_value,
    double max_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
    , m_current(default_value)
{}

std::string FloatingPointOption::set(double x){
    std::string err = check_validity(x);
    if (err.empty()){
        m_current.store(x, std::memory_order_relaxed);
        push_update();
    }
    return err;
}

void FloatingPointOption::load_json(const JsonValue& json){
    double value;
    if (!json.read_float(value)){
        return;
    }
    value = std::max(value, m_min_value);
    value = std::min(value, m_max_value);
    if (std::isnan(value)){
        value = m_default;
    }
    m_current.store(value, std::memory_order_relaxed);
    push_update();
}
JsonValue FloatingPointOption::to_json() const{
    return (double)*this;
}

std::string FloatingPointOption::check_validity(double x) const{
    if (x < m_min_value){
        std::ostringstream ss;
        return "Value too small: min = " + tostr_default(m_min_value) + ", value = " + tostr_default(x);
    }
    if (x > m_max_value){
        std::ostringstream ss;
        return "Value too large: max = " + tostr_default(m_max_value) + ", value = " + tostr_default(x);
    }
    if (std::isnan(x)){
        return "Value is NaN.";
    }
    return std::string();
}
std::string FloatingPointOption::check_validity() const{
    return check_validity(*this);
}
void FloatingPointOption::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
    push_update();
}







}
