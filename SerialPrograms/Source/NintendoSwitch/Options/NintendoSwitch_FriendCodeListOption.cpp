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
void FriendCodeListOption::load_json(const JsonValue& json){
    //  Legacy format: an array of lines.
    const JsonArray* array = json.to_array();
    if (array != nullptr){
        std::string text;
        for (const JsonValue& item : *array){
            const std::string* str = item.to_string();
            if (str == nullptr){
                continue;
            }
            if (!text.empty()){
                text += '\n';
            }
            text += *str;
        }
        set(std::move(text));
        return;
    }
    TextEditOption::load_json(json);
}



}
}
