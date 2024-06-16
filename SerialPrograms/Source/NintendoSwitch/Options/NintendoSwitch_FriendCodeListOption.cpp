/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "NintendoSwitch_FriendCodeListOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



FriendCodeListOption::FriendCodeListOption(std::string label, std::vector<std::string> default_lines)
    : m_label(std::move(label))
    , m_default(std::move(default_lines))
    , m_lines(m_default)
{}

void FriendCodeListOption::load_json(const JsonValue& json){
    const JsonArray* list = json.get_array();
    if (list == nullptr){
        return;
    }
    std::vector<std::string> lines;
    for (const auto& line : *list){
        const std::string* str = line.get_string();
        if (str == nullptr || str->empty()){
            continue;
        }
        lines.emplace_back(*str);
    }
    {
        SpinLockGuard lg(m_lock);
        m_lines = std::move(lines);
    }
    report_value_changed(this);
}
JsonValue FriendCodeListOption::to_json() const{
    JsonArray list;
    SpinLockGuard lg(m_lock);
    for (const std::string& line : m_lines){
        list.push_back(line);
    }
    return list;
}
void FriendCodeListOption::restore_defaults(){
    {
        SpinLockGuard lg(m_lock);
        m_lines = m_default;
    }
    report_value_changed(this);
}



std::vector<uint8_t> FriendCodeListOption::parse(const std::string& line){
    std::vector<uint8_t> code;
    for (char ch : line){
        if ('0' <= ch && ch <= '9'){
            code.emplace_back(ch);
        }
    }
    return code;
}
void FriendCodeListOption::set(const std::string& text){
    std::vector<std::string> lines;
    std::string line;
    for (char ch : text){
        if (ch == '\n'){
            lines.emplace_back(std::move(line));
            line.clear();
            continue;
        }
        line += ch;
    }
    if (line.size() > 0){
        lines.emplace_back(std::move(line));
    }
    {
        SpinLockGuard lg(m_lock);
        m_lines = std::move(lines);
    }
    report_value_changed(this);
}
std::vector<std::string> FriendCodeListOption::lines() const{
    SpinLockGuard lg(m_lock);
    return m_lines;
}
std::vector<std::string> FriendCodeListOption::list() const{
    SpinLockGuard lg(m_lock);
    std::vector<std::string> ret;
    for (const auto& item : m_lines){
        std::vector<uint8_t> line = parse(item);
        if (line.size() != 12){
            continue;
        }
        std::string str = "SW";
        for (size_t c = 0; c < 12; c++){
            if (c % 4 == 0){
                str += "-";
            }
            str += line[c];
        }
        ret.emplace_back(std::move(str));
    }
    return ret;
}





}
}









