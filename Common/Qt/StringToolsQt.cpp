/*  String Tools (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include <QString>
#include "StringToolsQt.h"

namespace PokemonAutomation{



std::u32string to_utf32(const std::string& str){
    return QString::fromStdString(str).toStdU32String();
}
std::string to_utf8(const std::u32string& str){
    return QString::fromStdU32String(str).toStdString();
}


std::string to_lower(const std::string& str){
    return QString::fromStdString(str).toLower().toStdString();
}
void to_lower(std::u32string& str){
    for (char32_t& ch : str){
        ch = QChar::toLower(ch);
    }
}

bool is_alphanumberic(char32_t ch){
    return QChar::isLetterOrNumber(ch);
}

bool has_extension(const std::string& str, const std::string& extension){
    if (extension.empty()){
        return true;
    }
    if (str.empty()){
        return false;
    }

    size_t c = str.size();
    while (c > 0){
        char ch = str[c - 1];
        if (ch == '.'){
            break;
        }
        c--;
    }
    std::string end = str.substr(c);
    return to_lower(end) == to_lower(extension);
}




}
