/*  Time
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Time_H
#define PokemonAutomation_Time_H

#include <chrono>
#include "Common/Compiler.h"

namespace PokemonAutomation{


using WallClock = std::chrono::system_clock::time_point;
using WallDuration = std::chrono::system_clock::duration;

constexpr PA_FORCE_INLINE WallDuration milliseconds_to_duration(int64_t milliseconds){
    return std::chrono::milliseconds(milliseconds);
}

PA_FORCE_INLINE bool wrapsafe_cmplt(WallClock x, WallClock y){
    return x < y;
}
PA_FORCE_INLINE bool wrapsafe_cmpgt(WallClock x, WallClock y){
    return x > y;
}
PA_FORCE_INLINE bool wrapsafe_cmple(WallClock x, WallClock y){
    return x <= y;
}
PA_FORCE_INLINE bool wrapsafe_cmpge(WallClock x, WallClock y){
    return x >= y;
}


using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;

inline WallClock current_time(){
    return std::chrono::system_clock::now();
}
std::string current_time_to_str();


uint16_t current_year();


}
#endif
