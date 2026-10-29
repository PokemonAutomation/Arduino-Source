/*  Raid/Trade Code Validator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CodeValidator.h"

namespace PokemonAutomation{


bool validate_code(size_t digits, const std::string& code){
    if (code.empty()){
        return true;
    }
    size_t c = 0;
    for (const auto& ch : code){
        if (ch == ' ' || ch == '-'){
            continue;
        }
        if (ch < '0'){
            return false;
        }
        if (ch > '9'){
            return false;
        }
        c++;
        if (c > digits){
            return false;
        }
    }
    return c == digits;
}

std::string sanitize_code(size_t digits, const std::string& code){
    std::string ret;
    size_t c = 0;
    for (const auto& ch : code){
        if (ch == ' ' || ch == '-'){
            continue;
        }
        if (ch < '0' || ch > '9'){
            throw ParseException(std::string("Invalid code digit: ") + ch);
        }
        c++;
        if (c > digits){
            throw ParseException(std::string("Code is too long: ") + code);
        }
        ret += ch;
    }
    if (c < digits){
        throw ParseException(std::string("Code is too short: ") + code);
    }
    return ret;
}


}
