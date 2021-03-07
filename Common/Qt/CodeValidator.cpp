/*  Raid/Trade Code Validator
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "StringException.h"
#include "CodeValidator.h"

namespace PokemonAutomation{


bool validate_code(size_t digits, const QString& code){
    if (code.isEmpty()){
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

QString sanitize_code(size_t digits, const QString& code){
    QString ret;
    size_t c = 0;
    for (const auto& ch : code){
        if (ch == ' ' || ch == '-'){
            continue;
        }
        if (ch < '0' || ch > '9'){
            throw StringException("Invalid code digit.");
        }
        c++;
        if (c > digits){
            throw StringException("Code is too long.");
        }
        ret += ch;
    }
    if (c < digits){
        throw StringException("Code is too short.");
    }
    return ret;
}


}
