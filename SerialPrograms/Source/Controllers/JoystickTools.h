/*  Joystick Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_JoystickTools_H
#define PokemonAutomation_Controllers_JoystickTools_H

#include <stdint.h>
#include <algorithm>

namespace PokemonAutomation{
namespace JoystickTools{



inline double linear_u8_to_float(uint8_t x){
    if (x <= 128){
        return (x - 128) * (1. / 128);
    }else{
        return (x - 128) * (1. / 127);
    }
}
inline uint8_t linear_float_to_u8(double f){
    if (f <= 0){
        f = std::max<double>(f, -1);
        f = f * 128 + 128;
        return (uint8_t)(f + 0.5);
    }else{
        f = std::min<double>(f, +1);
        f = f * 127 + 128;
        return (uint8_t)(f + 0.5);
    }
}
inline int16_t linear_float_to_s16(double f){
    if (f <= 0){
        f = std::max<double>(f, -1);
        f = f * 32768;
        return (int16_t)(f + 0.5);
    }else{
        f = std::min<double>(f, +1);
        f = f * 32767;
        return (int16_t)(f + 0.5);
    }
}
inline uint16_t linear_float_to_u16(double f){
    if (f <= 0){
        f = std::max<double>(f, -1);
        f = f * 32768 + 32768;
        return (uint16_t)(f + 0.5);
    }else{
        f = std::min<double>(f, +1);
        f = f * 32767 + 32768;
        return (uint16_t)(f + 0.5);
    }
}
inline uint16_t linear_float_to_u12(double lo, double hi, double f){
    if (f == 0){
        return 2048;
    }else if (f <= 0){
        f = std::max<double>(f, -1);
//        if (f < -1){ return 0; }
        f = f * (hi - lo) - lo;
        f = f * 2048 + 2048;
        return (uint16_t)(f + 0.5);
    }else{
        f = std::min<double>(f, +1);
//        if (f > 1){ return 4095; }
        f = f * (hi - lo) + lo;
        f = f * 2047 + 2048;
        return (uint16_t)(f + 0.5);
    }
}



}
}
#endif
