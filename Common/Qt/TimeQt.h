/*  Time Tools (Qt)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_TimeQt_H
#define PokemonAutomation_TimeQt_H

#include <string>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{


std::string to_utc_time_str(WallClock time);
WallClock parse_utc_time_str(const std::string& str);



}
#endif
