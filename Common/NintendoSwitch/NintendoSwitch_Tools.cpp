/*  Nintendo Switch Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NintendoSwitch_Tools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


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
}
