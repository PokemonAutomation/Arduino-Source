/*  Pretty Printing
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include <iomanip>
#include "Common/SwitchFramework/SwitchControllerDefs.h"
#include "PrettyPrint.h"

namespace PokemonAutomation{


std::string tostr_u_commas(uint64_t x){
    //  Prints out x with comma separators.

    std::string str = std::to_string(x);
    std::string out;

    const char* ptr = str.c_str();
    size_t len = str.size();

    size_t commas = (len + 2) / 3 - 1;
    size_t shift = len - commas * 3;

    while (1){
        char ch = *ptr++;
        if (ch == '\0')
            break;
        if (shift == 0){
            out += ',';
            shift = 3;
        }
        out += ch;
        shift--;
    }

    return out;
}
std::string tostr_fixed(double x, int precision){
    std::ostringstream out;
    out << std::setprecision(precision);
    out << std::fixed;
    out << x;
    return out.str();
}
std::string ticks_to_time(uint64_t ticks){
    const uint64_t SECOND = TICKS_PER_SECOND;
    const uint64_t MINUTE = SECOND * 60;
    const uint64_t HOUR = MINUTE * 60;
    const uint64_t DAY = HOUR * 24;

    std::string str;
    str += tostr_u_commas(ticks);
    str += " tick";
    if (ticks != 1){
        str += "s";
    }
    str += " (";
    if (ticks < MINUTE * 2){
        str += tostr_fixed((double)ticks / SECOND, 3);
        str += " seconds";
    }else if (ticks < HOUR * 2){
        str += tostr_fixed((double)ticks / MINUTE, 3);
        str += " minutes";
    }else if (ticks < DAY * 2){
        str += tostr_fixed((double)ticks / HOUR, 3);
        str += " hours";
    }else{
        str += tostr_fixed((double)ticks / DAY, 3);
        str += " days";
    }
    str += ")";
    return str;
}




}
