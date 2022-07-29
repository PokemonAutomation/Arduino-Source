/*  String Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "StringTools.h"

namespace PokemonAutomation{
namespace StringTools{


std::string replace(const std::string& str, const std::string& desired, const std::string& replace_with){
    std::string ret;
    size_t index = 0;
    while (index < str.size()){
        size_t pos = str.find(desired, index);
        if (pos == std::string::npos){
            break;
        }
        ret += str.substr(index, pos - index);
        ret += replace_with;
        index = pos + desired.size();
    }
    ret += str.substr(index);
    return ret;;
}




}
}
