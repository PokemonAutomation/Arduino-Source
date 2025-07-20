/*  Joystick Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_JoystickTools_H
#define PokemonAutomation_Controllers_JoystickTools_H

#include <stdint.h>
#include <cmath>
#include <algorithm>

namespace PokemonAutomation{
namespace JoystickTools{


inline void clip_magnitude(double& x, double& y){
    double mag = x*x + y*y;
    if (mag <= 1.0){
        return;
    }

    double scale = 1 / std::sqrt(mag);

    x *= scale;
    y *= scale;
}
inline void max_out_magnitude(double& x, double& y){
    double mag = x*x + y*y;
    if (mag == 0){
        return;
    }

    double abs_x = std::abs(x);
    double abs_y = std::abs(y);

    if (abs_x < abs_y){
        x /= abs_y;
        y = y < 0 ? -1 : 1;
    }else{
        y /= abs_x;
        x = x < 0 ? -1 : 1;
    }
}
inline double project_to_range(double x, double lo, double hi){
    if (x <= -1){
        return -1;
    }
    if (x >= 1){
        return 1;
    }
    return x * (hi - lo) + lo;
}




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



inline uint16_t linear_float_to_u12(double f){
    if (f <= 0){
        f = std::max<double>(f, -1);
        f = f * 2048 + 2048;
        return (uint16_t)(f + 0.5);
    }else{
        f = std::min<double>(f, +1);
        f = f * 2047 + 2048;
        return (uint16_t)(f + 0.5);
    }
}








}
}
#endif
