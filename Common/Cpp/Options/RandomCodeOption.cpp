/*  Random Code Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <time.h>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/CodeValidator.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "RandomCodeOption.h"

namespace PokemonAutomation{



RaidCodeOption::RaidCodeOption(size_t total_digits)
    : m_digits(total_digits)
    , m_random_digits(0)
    , m_code(std::string(total_digits, '1'))
{}
RaidCodeOption::RaidCodeOption(size_t total_digits, size_t random_digits, std::string code_string)
    : m_digits(total_digits)
    , m_random_digits(random_digits)
    , m_code(std::move(code_string))
{}
std::string RaidCodeOption::check_validity() const{
    if (m_random_digits == 0){
        return validate_code(m_digits, m_code)
            ? std::string()
            : "Code is invalid.";
    }else{
        return m_random_digits <= m_digits
            ? std::string()
            : "Random digits cannot be greater than " + std::to_string(m_digits) + ".";
    }
}
bool RaidCodeOption::code_enabled() const{
    return m_random_digits != 0 || m_code.size() > 0;
}
bool RaidCodeOption::get_code(uint8_t* code) const{
    if (!code_enabled()){
        return false;
    }
    if (m_random_digits == 0){
        std::string qstr = sanitize_code(8, m_code);
        for (int c = 0; c < 8; c++){
            code[c] = qstr[c] - '0';
        }
        return true;
    }
    srand((unsigned)time(nullptr));
    for (uint8_t c = 0; c < m_random_digits; c++){
        uint8_t x;
        do{
            x = rand() & 0x0f;
        }while (x >= 10);
        code[c] = x;
    }
    for (size_t c = m_random_digits; c < m_digits; c++){
        code[c] = code[c - 1];
    }
    return true;
}


RandomCodeOption::RandomCodeOption(size_t total_digits)
    : m_label(
        "<b>Raid Code:</b><br>Blank for no raid code. Set random digits to zero for a fixed code. Otherwise, it is the # of leading random digits."
    )
    , m_default(total_digits)
    , m_current(m_default)
{}
RandomCodeOption::RandomCodeOption(std::string label, size_t total_digits, size_t random_digits, std::string code_string)
    : m_label(std::move(label))
    , m_default(total_digits, random_digits, std::move(code_string))
    , m_current(m_default)
{}
#if 0
std::unique_ptr<ConfigOption> RandomCodeOption::clone() const{
    std::unique_ptr<RandomCodeOption> ret(new RandomCodeOption(
        m_label,
        m_default.total_digits(),
        m_default.random_digits(),
        m_default.code_string()
    ));
    ret->m_current = m_current;
    return ret;
}
#endif
void RandomCodeOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }

    RaidCodeOption code(m_default.total_digits());
    obj->read_integer(m_current.m_random_digits, "RandomDigits");

    std::string str;
    if (obj->read_string(str, "RaidCode")){
        code.m_code = str;
    }

    {
        SpinLockGuard lg(m_lock);
        m_current = code;
    }
    push_update();
}
JsonValue RandomCodeOption::to_json() const{
    JsonObject obj;
    obj["RandomDigits"] = m_current.m_random_digits;
    obj["RaidCode"] = m_current.m_code;
    return obj;
}



RandomCodeOption::operator RaidCodeOption() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
std::string RandomCodeOption::set(RaidCodeOption code){
//    std::string error = code.check_validity();
//    if (!error.empty()){
//        return error;
//    }
    {
        SpinLockGuard lg(m_lock);
        m_current = code;
    }
    push_update();
    return std::string();
}
bool RandomCodeOption::code_enabled() const{
    SpinLockGuard lg(m_lock);
    return m_current.code_enabled();
}
bool RandomCodeOption::get_code(uint8_t* code) const{
    SpinLockGuard lg(m_lock);
    return m_current.get_code(code);
}

std::string RandomCodeOption::check_validity() const{
    return m_current.check_validity();
}
void RandomCodeOption::restore_defaults(){
    {
        SpinLockGuard lg(m_lock);
        m_current = m_default;
    }
    push_update();
}





}









