/*  Time
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Time_H
#define PokemonAutomation_Time_H

#include <chrono>

namespace PokemonAutomation{


using WallClock = std::chrono::system_clock::time_point;
using WallDuration = std::chrono::system_clock::duration;

using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;

inline WallClock current_time(){
    return std::chrono::system_clock::now();
}


uint16_t current_year();


}
#endif
