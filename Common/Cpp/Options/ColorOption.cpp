/*  Color Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
 #include "ColorOption.h"

namespace PokemonAutomation{


ColorCell::ColorCell(const ColorCell& x)
    : ConfigOption(x)
    , m_default_value(x.m_default_value)
    , m_current_value(x)
{}
ColorCell::ColorCell(
    LockMode lock_while_running,
    bool has_alpha,
    uint32_t default_value, uint32_t current_value
)
    : ConfigOption(lock_while_running)
    , m_has_alpha(has_alpha)
    , m_default_value(default_value)
    , m_current_value(current_value)
{
    if (!m_has_alpha){
        m_default_value &= 0x00ffffff;
        m_current_value &= 0x00ffffff;
    }
}
uint32_t ColorCell::default_value() const{
    return m_default_value;
}
std::string ColorCell::to_str() const{
    return color_to_str(*this, m_has_alpha);
}
std::string ColorCell::color_to_str(uint32_t color, bool has_alpha){
    static const char HEX_DIGITS[] = "0123456789ABCDEF";
    std::string str;
    if (has_alpha){
        str += HEX_DIGITS[(color >> 28) & 0xf];
        str += HEX_DIGITS[(color >> 24) & 0xf];
    }
    str += HEX_DIGITS[(color >> 20) & 0xf];
    str += HEX_DIGITS[(color >> 16) & 0xf];
    str += HEX_DIGITS[(color >> 12) & 0xf];
    str += HEX_DIGITS[(color >>  8) & 0xf];
    str += HEX_DIGITS[(color >>  4) & 0xf];
    str += HEX_DIGITS[(color >>  0) & 0xf];
    return str;
}
ColorCell::operator uint32_t() const{
    return m_current_value.load(std::memory_order_relaxed);
}
void ColorCell::set(uint32_t x){
    if (!m_has_alpha){
        x &= 0x00ffffff;
    }
    if (x != m_current_value.exchange(x, std::memory_order_relaxed)){
        report_value_changed(this);
    }
}
void ColorCell::set(const std::string& str){
    size_t max_digits = m_has_alpha ? 8 : 6;

    uint32_t x = 0;
    size_t count = 0;
    for (char ch : str){
        if (count >= max_digits){
            break;
        }
        switch (ch){
        case '\0':
            break;
        case '-':
        case ':':
        case ' ':
            continue;
        }

        if ('0' <= ch && ch <= '9'){
            x <<= 4;
            x |= ch - '0';
            count++;
            continue;
        }
        if ('a' <= ch && ch <= 'f'){
            x <<= 4;
            x |= ch - 'a' + 10;
            count++;
            continue;
        }
        if ('A' <= ch && ch <= 'F'){
            x <<= 4;
            x |= ch - 'A' + 10;
            count++;
            continue;
        }

        break;
    }
    if (!m_has_alpha){
        x &= 0x00ffffff;
    }
    set(x);
}

void ColorCell::load_json(const JsonValue& json){
    uint32_t value;
    if (!json.read_integer(value)){
        return;
    }
    set(value);
}
JsonValue ColorCell::to_json() const{
    return (uint32_t)*this;
}
void ColorCell::restore_defaults(){
    set(m_default_value);
}



}
