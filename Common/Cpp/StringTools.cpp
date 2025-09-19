/*  String Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <stdexcept>
#include <vector>
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

std::string strip(const std::string& str){
    size_t first = str.find_first_not_of(" \t\n\r"); // Find first non-whitespace character
    if (std::string::npos == first) { // If the string is all whitespace or empty
        return ""; // Return an empty string
    }
    size_t last = str.find_last_not_of(" \t\n\r"); // Find last non-whitespace character
    return str.substr(first, (last - first + 1)); // Extract the trimmed substring
}

size_t to_size_t(const std::string& str){
    try {
        int num = std::stoi(str);
        return static_cast<size_t>(num);
    } catch (const std::invalid_argument&) {
        return SIZE_MAX;
    } catch (const std::out_of_range&) {
        return SIZE_MAX;
    }
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    
    tokens.push_back(str.substr(start));
    return tokens;
}


}
}
