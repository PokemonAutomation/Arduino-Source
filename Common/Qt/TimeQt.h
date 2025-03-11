/*  Time Tools (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_TimeQt_H
#define PokemonAutomation_TimeQt_H

#include <string>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/DateTime.h"

namespace PokemonAutomation{


std::string to_utc_time_str(WallClock time);
WallClock parse_utc_time_str(const std::string& str);

int64_t to_seconds_since_epoch(const DateTime& date);
DateTime from_seconds_since_epoch(int64_t seconds_since_epoch);


}
#endif
