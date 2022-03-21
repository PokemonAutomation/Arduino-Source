/*  Nintendo Switch Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NintendoSwitch_Tools.h"
#include <limits>

namespace PokemonAutomation{
namespace NintendoSwitch{


std::string ticks_to_time(int64_t ticks){
    const uint64_t SECOND = TICKS_PER_SECOND;
    const uint64_t MINUTE = SECOND * 60;
    const uint64_t HOUR = MINUTE * 60;
    const uint64_t DAY = HOUR * 24;

    std::string str;
    str += tostr_u_commas(ticks);
    str += " tick";
    // Compute absolute value of the ticks:
    uint64_t abs_ticks = 0;
    // int64_t range from [-2^63, 2^63-1], so if ticks is -2^63, abs(ticks) will overflow.
    // This if-statement handles this case.
    if (ticks == std::numeric_limits<int64_t>::min()){
        abs_ticks = uint64_t(std::numeric_limits<int64_t>::max()) + 1;
    } else{
        abs_ticks = std::abs(ticks);
    }
    if (abs_ticks != 1){
        str += "s";
    }
    str += " (";
    if (ticks < 0){
        str += "-";
    }
    if (abs_ticks < MINUTE * 2){
        str += tostr_fixed((double)abs_ticks / SECOND, 3);
        str += " seconds";
    }else if (abs_ticks < HOUR * 2){
        str += tostr_fixed((double)abs_ticks / MINUTE, 3);
        str += " minutes";
    }else if (abs_ticks < DAY * 2){
        str += tostr_fixed((double)abs_ticks / HOUR, 3);
        str += " hours";
    }else{
        str += tostr_fixed((double)abs_ticks / DAY, 3);
        str += " days";
    }
    str += ")";
    return str;
}


}
}
