/*  Random Code Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <time.h>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/CodeValidator.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "RandomCodeOption.h"

namespace PokemonAutomation{



RaidCodeOption::~RaidCodeOption() = default;
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
std::string RaidCodeOption::get_code() const{
    if (!code_enabled()){
        return "";
    }
    std::string code;
    code.resize(8);
    if (m_random_digits == 0){
        std::string qstr = sanitize_code(8, m_code);
        for (int c = 0; c < 8; c++){
            code[c] = qstr[c];
        }
        return code;
    }
    srand((unsigned)time(nullptr));
    for (uint8_t c = 0; c < m_random_digits; c++){
        uint8_t x;
        do{
            x = rand() & 0x0f;
        }while (x >= 10);
        code[c] = x + '0';
    }
    for (size_t c = m_random_digits; c < m_digits; c++){
        code[c] = code[c - 1];
    }
    return code;
}
bool RaidCodeOption::operator==(const RaidCodeOption& x) const{
    return
        m_digits == x.m_digits &&
        m_random_digits == x.m_random_digits &&
        m_code == x.m_code;
}


struct RandomCodeOption::Data{
    const std::string m_label;
    const RaidCodeOption m_default;

    mutable SpinLock m_lock;
    RaidCodeOption m_current;

    Data(size_t total_digits)
        : m_label(
            "<b>Raid Code:</b><br>Blank for no raid code. Set random digits to zero for a fixed code. Otherwise, it is the # of leading random digits."
        )
        , m_default(total_digits)
        , m_current(m_default)
    {}
    Data(std::string label, size_t total_digits, size_t random_digits, std::string code_string)
        : m_label(std::move(label))
        , m_default(total_digits, random_digits, std::move(code_string))
        , m_current(m_default)
    {}
};



RandomCodeOption::~RandomCodeOption() = default;
RandomCodeOption::RandomCodeOption(size_t total_digits)
    : m_data(CONSTRUCT_TOKEN, total_digits)
{}
RandomCodeOption::RandomCodeOption(std::string label, size_t total_digits, size_t random_digits, std::string code_string)
    : m_data(CONSTRUCT_TOKEN, std::move(label), total_digits, random_digits, std::move(code_string))
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

const std::string& RandomCodeOption::label() const{
    return m_data->m_label;
}

void RandomCodeOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    RaidCodeOption code(m_data->m_default.total_digits());

    obj->read_integer(code.m_random_digits, "RandomDigits");

    std::string str;
    if (obj->read_string(str, "RaidCode")){
        code.m_code = str;
    }

    {
        WriteSpinLock lg(m_data->m_lock);
        m_data->m_current = code;
    }
    report_value_changed(this);
}
JsonValue RandomCodeOption::to_json() const{
    ReadSpinLock lg(m_data->m_lock);
    JsonObject obj;
    obj["RandomDigits"] = m_data->m_current.m_random_digits;
    obj["RaidCode"] = m_data->m_current.m_code;
    return obj;
}



RandomCodeOption::operator RaidCodeOption() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current;
}
std::string RandomCodeOption::set(RaidCodeOption code){
    std::string error = code.check_validity();
    if (!error.empty()){
        return error;
    }
    {
        WriteSpinLock lg(m_data->m_lock);
        if (m_data->m_current == code){
            return std::string();
        }
        m_data->m_current = code;
    }
    report_value_changed(this);
    return std::string();
}
bool RandomCodeOption::code_enabled() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current.code_enabled();
}
std::string RandomCodeOption::get_code() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current.get_code();
}

std::string RandomCodeOption::check_validity() const{
    return m_data->m_current.check_validity();
}
void RandomCodeOption::restore_defaults(){
    {
        WriteSpinLock lg(m_data->m_lock);
        m_data->m_current = m_data->m_default;
    }
    report_value_changed(this);
}





}









