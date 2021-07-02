/*  Raid/Trade Code Validator
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
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
            PA_THROW_ParseException(std::string("Invalid code digit: ") + QString(ch).toUtf8().data());
        }
        c++;
        if (c > digits){
            PA_THROW_ParseException(std::string("Code is too long: ") + code.toUtf8().data());
        }
        ret += ch;
    }
    if (c < digits){
        PA_THROW_ParseException(std::string("Code is too short: ") + code.toUtf8().data());
    }
    return ret;
}


}
