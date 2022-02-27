/*  Raid/Trade Code Validator
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
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
            throw ParseException(std::string("Invalid code digit: ") + QString(ch).toStdString());
        }
        c++;
        if (c > digits){
            throw ParseException(std::string("Code is too long: ") + code.toStdString());
        }
        ret += ch;
    }
    if (c < digits){
        throw ParseException(std::string("Code is too short: ") + code.toStdString());
    }
    return ret;
}


}
