/*  Time
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Time_H
#define PokemonAutomation_Time_H

#include <chrono>

namespace PokemonAutomation{


using WallClock = std::chrono::system_clock::time_point;
using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;

inline WallClock current_time(){
    return std::chrono::system_clock::now();
}



}
#endif
