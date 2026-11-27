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
#include "Joystick.h"

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

//  Maps a joystick position into a Switch console's report range.
//
//  The thresholds are fractions of full scale (0..1, where 1 is
//  the full travel from neutral):
//
//      min_threshold: the point at which the console's joystick
//          calibrator registers the stick as off-center (Nintendo's
//          way of alleviating the joycon drift problem): a moved
//          stick reports at 1 - min_threshold in the direction of
//          movement rather than near neutral.
//
//      max_threshold: where the stick reaches its maximum value:
//          a genuine controller does not use the full range, and
//          saturates at 1 - max_threshold.
//
//  A non-neutral position's magnitude is mapped linearly between
//  the two, an exactly neutral position stays neutral, and a
//  position beyond full travel is clamped to the edge of the unit
//  square.
inline JoystickPosition map_to_range(
    double min_threshold, double max_threshold,
    const JoystickPosition& position
){
    double fx = position.x;
    double fy = position.y;
    double mag_squared = fx*fx + fy*fy;

    if (mag_squared == 0){
        return position;
    }

    fx = std::min(fx, +1.0);
    fx = std::max(fx, -1.0);
    fy = std::min(fy, +1.0);
    fy = std::max(fy, -1.0);

    if (mag_squared >= 1){
        max_out_magnitude(fx, fy);
        return JoystickPosition(fx, fy);
    }

    double lo = 1 - min_threshold;
    double hi = 1 - max_threshold;

    double true_mag = std::sqrt(mag_squared);
    double report_mag = project_to_range(true_mag, lo, hi);
    double scale = report_mag / true_mag;

    return JoystickPosition(fx * scale, fy * scale);
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
