/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "NintendoSwitch_FriendCodeListOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


namespace{
std::string join_lines(const std::vector<std::string>& lines){
    std::string ret;
    for (const std::string& line : lines){
        if (!ret.empty()){
            ret += '\n';
        }
        ret += line;
    }
    return ret;
}
}


FriendCodeListOption::FriendCodeListOption(std::string label, std::vector<std::string> default_lines)
    : TextEditOption(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        join_lines(default_lines),
        ""
    )
{}


std::string FriendCodeListOption::parse(const std::string& line){
    std::string code;
    for (char ch : line){
        if ('0' <= ch && ch <= '9'){
            code.push_back(ch);
        }
    }
    return code;
}
std::vector<std::string> FriendCodeListOption::lines() const{
    const std::string text = *this;
    std::vector<std::string> ret;
    std::string line;
    for (char ch : text){
        if (ch == '\n'){
            ret.emplace_back(std::move(line));
            line.clear();
            continue;
        }
        line += ch;
    }
    if (line.size() > 0){
        ret.emplace_back(std::move(line));
    }
    return ret;
}
std::vector<std::string> FriendCodeListOption::list() const{
    std::vector<std::string> ret;
    for (const std::string& item : lines()){
        std::string line = parse(item);
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
JsonValue FriendCodeListOption::to_json() const{
    JsonArray list;
    for (const std::string& line : lines()){
        list.push_back(line);
    }
    return list;
}
void FriendCodeListOption::load_json(const JsonValue& json){
    //  The settings are stored as an array of lines, one entry per line.
    const JsonArray* list = json.to_array();
    if (list == nullptr){
        return;
    }
    std::vector<std::string> lines;
    for (const auto& line : *list){
        const std::string* str = line.to_string();
        if (str == nullptr || str->empty()){
            continue;
        }
        lines.emplace_back(*str);
    }
    set(join_lines(lines));
}



}
}
